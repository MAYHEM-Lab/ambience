from __future__ import annotations

import collections
from typing import Dict, Set, List
import os
import subprocess
import abc
from toposort import toposort, toposort_flatten

from jinja2 import Environment, PackageLoader, select_autoescape

env = Environment(
    loader=PackageLoader("ambictl"),
    autoescape=select_autoescape()
)


class LidlModule:
    _abs_path: str
    cmake_target: str

    def __init__(self, abs_path: str, cmake_target: str):
        self._abs_path = abs_path
        self.cmake_target = cmake_target

    def absolute_import(self):
        return self._abs_path

    def file_name(self):
        return os.path.basename(self._abs_path)

    def get_service(self, name: str) -> ServiceInterface:
        return ServiceInterface(self, name)


class ServiceInterface:
    module: LidlModule
    full_serv_name: str

    def __init__(self, mod: LidlModule, full_name: str) -> None:
        self.module = mod
        self.full_serv_name = full_name

    def absolute_name(self):
        return self.full_serv_name

    def sync_server_name(self):
        return f"{self.absolute_name()}::sync_server"

    def async_server_name(self):
        return f"{self.absolute_name()}::async_server"

    def sync_stub_client(self):
        return f"{self.absolute_name()}::stub_client"

    def get_include(self):
        return os.path.splitext(self.module.file_name())[0] + "_generated.hpp"

    def implement(self, name: str, *, sync: bool,
                  deps: Dict[str, ServiceInterface] = {}, cmake_target: str = "") -> Service:
        return Service(name, cmake_target, self, sync, deps)


class Service:
    name: str
    cmake_target: str
    iface: ServiceInterface

    # Whether a service is async/sync is only important if both the client and the server is on the same group.
    # For instance, if a sync service is running in user space but the client is in the kernel, the kernel will still
    # see an async interface.
    sync: bool

    deps: Dict[str, ServiceInterface]

    def __init__(self, name: str, cmake_target: str, iface: ServiceInterface, sync: bool,
                 deps: Dict[str, ServiceInterface] = {}) -> None:
        self.name = name
        self.cmake_target = cmake_target
        self.iface = iface
        self.deps = deps
        self.sync = sync

    def cxx_init_signature(self) -> str:
        params = (f"{val.sync_server_name() if self.sync else val.async_server_name()}* {key}" for key, val in
                  self.deps.items())
        if self.sync:
            return f"auto init_{self.name}({', '.join(params)}) -> {self.server_name()}*;"
        else:
            return f"auto init_{self.name}({', '.join(params)}) -> tos::Task<{self.server_name()}*>;"

    def instantiate(self, name: str, deps: Dict[str, Instance] = {}) -> ServiceInstance:
        return ServiceInstance(name, self, deps)

    def server_name(self):
        if self.sync:
            return self.iface.sync_server_name()
        else:
            return self.iface.async_server_name()


class Export:
    exporter: Exporter
    instance: Instance
    config: {}

    def __init__(self, exporter, instance, config):
        self.exporter = exporter
        self.instance = instance
        self.config = config

    def cxx_export_string(self):
        return self.exporter.export_service_string(self)


class Instance:
    name: str
    exports: Dict[Exporter, Export]
    extern: bool

    @abc.abstractmethod
    def get_interface(self) -> ServiceInterface:
        raise NotImplementedError()

    def __init__(self, name: str):
        self.name = name
        self.exports = {}
        self.extern = False

    def export(self, exporter, config=None):
        if exporter not in self.exports:
            print(f"Export {self.name} with {exporter}")
            self.exports[exporter] = exporter.export(self, config)
        return self.exports[exporter]

    def needs_init(self):
        return not self.extern

    @abc.abstractmethod
    def get_dependencies(self):
        raise NotImplementedError()

    @abc.abstractmethod
    def registry_type(self):
        raise NotImplementedError()


class ServiceInstance(Instance):
    impl: Service
    deps: Dict[str, Instance]

    assigned_group: Group

    def unmetDeps(self):
        return [nm for nm, dep in self.deps.items() if dep is None]

    def __init__(self, name: str, impl: Service, deps: Dict[str, Instance] = {}) -> None:
        super().__init__(name)
        self.impl = impl
        self.deps = deps
        self.assigned_group = None

        for nm, dep in self.impl.deps.items():
            if nm not in self.deps:
                self.deps[nm] = None

    def registry_type(self):
        # if not self.assigned_group.privileged:
        #     return self.impl.iface.async_server_name()

        return self.impl.server_name()

    def get_interface(self) -> ServiceInterface:
        return self.impl.iface

    def get_dependencies(self):
        return self.impl.deps.values()


class ImportedService(Instance):
    _import: Import

    def __init__(self, name: str, imprt: Import):
        super().__init__(name)
        self._import = imprt

    def get_interface(self) -> ServiceInterface:
        return self._import.interface

    def get_dependencies(self):
        return []

    def registry_type(self):
        return self._import.interface.sync_server_name()


class ExternService(Instance):
    iface: ServiceInterface
    sync: bool

    def __init__(self, name: str, iface: ServiceInterface, sync: bool):
        super().__init__(name)
        self.iface = iface
        self.extern = True
        self.sync = sync

    def get_interface(self) -> ServiceInterface:
        return self.iface

    def get_dependencies(self):
        return []

    def registry_type(self):
        if self.sync:
            return self.iface.sync_server_name()
        return self.iface.async_server_name()


class Group:
    name: str
    servs: Set[Instance]
    dg: DeployGroup

    def __init__(self, name: str, servs: Set[Instance]) -> None:
        self.name = name
        self.servs = servs
        for serv in self.servs:
            serv.assigned_group = self

    def servsWithUnmetDeps(self):
        return [serv for serv in self.servs if len(serv.unmetDeps()) != 0]

    def uniqueDeps(self) -> Set[Instance]:
        return set(dep for serv in self.servs for dep in serv.deps.values())

    def _externalDeps(self):
        return (dep for dep in self.uniqueDeps() if dep not in self.servs)

    def uniqueExternalDeps(self):
        return set(self._externalDeps())

    def interfaceDeps(self):
        all_ifaces = {serv.get_interface(): set(serv.get_dependencies()) for serv in self.servs}
        return toposort_flatten(all_ifaces, sort=False)

    def generateInitSigSection(self):
        unique_impls = set(s.impl for s in self.servs if isinstance(s, ServiceInstance))
        return "\n".join(s.cxx_init_signature() for s in unique_impls)

    @abc.abstractmethod
    def generate_group_dir(self, build_root):
        raise NotImplementedError()

    @abc.abstractmethod
    def generate_loader_dir(self, build_root):
        raise NotImplementedError()

    @abc.abstractmethod
    def post_generation(self, build_root, conf_dirs):
        pass

    @abc.abstractmethod
    def post_generation2(self, build_root, conf_dirs):
        pass


class DeployGroup:
    node: DeployNode
    group: Group
    heap_size: int
    queue_size: int
    sizes: (int, int)
    memories: Memories
    entry_point: int

    def __init__(self, node: DeployNode, group: Group, heap_size: int, queue_size: int):
        group.dg = self
        self.node = node
        self.group = group
        self.heap_size = heap_size
        self.queue_size = queue_size
        self.memories = None
        self.sizes = None
        self.entry_point = 0


class Memories:
    rom: (int, int)
    ram: (int, int)

    def __init__(self, rom, ram):
        self.rom = rom
        self.ram = ram


class GroupLoader:
    def generateGroupLoader(self, group: DeployGroup):
        raise NotImplementedError()


class Import:
    importer: Importer
    interface: ServiceInterface
    config: {}

    def __init__(self, importer: Importer, iface: ServiceInterface, config):
        self.importer = importer
        self.interface = iface
        self.config = config

    def cxx_import_string(self):
        return self.importer.import_string(self)

    def cxx_include(self):
        return self.importer.get_cxx_include()

    def instantiate(self, name: str):
        return ImportedService(name, self)


class Importer:
    @abc.abstractmethod
    def make_import(self, service: ServiceInterface, config) -> Import:
        pass

    @abc.abstractmethod
    def import_from(self, export: Export) -> Import:
        pass

    @abc.abstractmethod
    def import_string(self, import_: Import):
        pass

    @abc.abstractmethod
    def get_cxx_include(self):
        pass


class Exporter:
    @abc.abstractmethod
    def export(self, service: Instance, config) -> Export:
        pass

    @abc.abstractmethod
    def export_service_string(self, export: Export):
        pass

    @abc.abstractmethod
    def get_cxx_include(self):
        pass


class LwipUdpImporter(Importer):
    def __init__(self):
        super().__init__()

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.impl.iface
        conf = {
            "ip": export.instance.assigned_group.dg.node.node.ip_address,
            "port": export.config
        }
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        format = "{}<tos::ae::udp_transport>{{tos::udp_endpoint_t{{tos::parse_ipv4_address(\"{}\"), {{{}}} }} }}"
        return format.format(import_.interface.sync_stub_client(), import_.config["ip"], import_.config["port"])

    def get_cxx_include(self):
        return "tos/ae/transport/lwip/udp.hpp"


class LwipUdpExporter(Exporter):
    allocation: Dict
    next: int

    def __init__(self):
        super().__init__()
        self.allocation = {}
        self.next = 1993

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::lwip_host(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), tos::port_num_t{{{export.config}}});"

    def get_cxx_include(self):
        return "tos/ae/transport/lwip/host.hpp"


class HostedUdpImporter(Importer):
    def __init__(self):
        super().__init__()

    def make_import(self, service, config) -> Import:
        return Import(self, service, config)

    def import_from(self, export: Export):
        iface = export.instance.impl.iface
        conf = {
            "ip": export.instance.assigned_group.dg.node.node.ip_address,
            "port": export.config
        }
        return self.make_import(iface, conf)

    def import_string(self, import_: Import):
        format = "{}<tos::ae::hosted_udp_transport>{{tos::udp_endpoint_t{{tos::parse_ipv4_address(\"{}\"), {{{}}} }} }}"
        return format.format(import_.interface.sync_stub_client(), import_.config["ip"], import_.config["port"])

    def get_cxx_include(self):
        return "tos/ae/transport/hosted/udp.hpp"


class HostedUdpExporter(Exporter):
    allocation: Dict
    next: int

    def __init__(self):
        super().__init__()
        self.allocation = {}
        self.next = 1993

    def get_port_for_service(self, service):
        if service not in self.allocation:
            self.allocation[service] = self.next
            self.next = self.next + 1
        return self.allocation[service]

    def export(self, service, config) -> Export:
        if config is None:
            config = self.get_port_for_service(service)
        return Export(self, service, config)

    def export_service_string(self, export):
        return f"new tos::ae::hosted_udp_host(tos::ae::service_host(co_await registry.wait<\"{export.instance.name}\">()), tos::port_num_t{{{export.config}}});"

    def get_cxx_include(self):
        return "tos/ae/transport/hosted/udp_host.hpp"


class Platform(abc.ABC):
    loader: GroupLoader

    def __init__(self, loader: GroupLoader):
        self.loader = loader

    @abc.abstractmethod
    def generateBuildDirectories(self, source_dir: str):
        raise NotImplementedError()

    def generateGroupLoader(self, group: DeployGroup):
        return self.loader.generateGroupLoader(group)

    @abc.abstractmethod
    def make_deploy_node(self, node: Node, groups: [Group]):
        raise NotImplementedError()

    def make_node(self, name: str, mems: Memories, exporters: [Exporter] = [], importers: [Importer] = []):
        return Node(name, self, mems, exporters, importers)


class Node:
    name: str
    platform: Platform
    memories: Memories
    exporters: [Exporter]
    importers: [Importer]
    ip_address: str

    def __init__(self, name: str, platform: Platform, memories: Memories, exporters: [Exporter], importers: [Importer]):
        self.name = name
        self.platform = platform
        self.memories = memories
        self.exporters = exporters
        self.importers = importers
        self.ip_address = "127.0.0.1"

    def deploy(self, groups: List[Group]):
        return self.platform.make_deploy_node(self, groups)


class DeployNode:
    node: Node
    groups: List[Group]
    deploy_groups: Dict[Group, DeployGroup]
    target_name: str

    def __init__(self, node: Node, groups: List[Group]):
        self.node = node
        self.groups = groups
        self.deploy_groups = {g: DeployGroup(self, g, 2048, 32) for g in self.groups}
        self.target_name = None

    def visibleServices(self):
        return (serv for group in self.groups for serv in group.servs)

    def generateRegistry(self):
        template = env.get_template("node/registry.hpp")
        return template.render({
            "services": {serv.name: serv.registry_type() for serv in self.visibleServices()},
            "service_includes": set(iface.get_include() for group in self.groups for iface in group.interfaceDeps())
        })

    def generate_node_dir(self, build_dir):
        for g in self.deploy_groups.keys():
            g.generate_group_dir(build_dir)

        node_dir = os.path.join(build_dir, self.node.name)
        loaders_dir = os.path.join(node_dir, "loaders")
        os.makedirs(loaders_dir, exist_ok=True)

        with open(os.path.join(node_dir, "CMakeLists.txt"), "w+") as node_cmake:
            template = env.get_template("node/CMakeLists.txt")
            node_cmake.write(template.render({
                "node_name": self.node.name,
                "node_groups": (g.name for g in self.deploy_groups.keys()),
                "schemas": set(iface.module.cmake_target for g in self.groups for iface in g.interfaceDeps())
            }))

        with open(os.path.join(loaders_dir, "CMakeLists.txt"), "w+") as groups_cmake:
            template = env.get_template("node/loaders/CMakeLists.txt")
            groups_cmake.write(template.render({
                "node_loaders": (g.name for g in self.deploy_groups.keys())
            }))

        with open(os.path.join(node_dir, "groups.hpp"), "w+") as node_src:
            template = env.get_template("node/groups.hpp")
            node_src.write(template.render({
                "groups": (group.name for group in self.deploy_groups),
                "group_includes": (f"{group.name}.hpp" for group in self.deploy_groups)
            }))

        with open(os.path.join(node_dir, "groups.cpp"), "w+") as node_src:
            template = env.get_template("node/groups.cpp")
            node_src.write(template.render({
                "groups": list(group.name for group in self.deploy_groups)
            }))

        with open(os.path.join(node_dir, "registry.hpp"), "w+") as node_src:
            node_src.write(self.generateRegistry())

        for dg in self.deploy_groups.values():
            group_dir = os.path.join(loaders_dir, dg.group.name)
            os.makedirs(group_dir, exist_ok=True)
            loader = dg.group.generate_loader_dir("")
            for name, content in loader.items():
                with open(os.path.join(group_dir, name), "w+") as group_src:
                    group_src.write(content)

    def post_generate(self, build_root, conf_dirs):
        for g in self.groups:
            g.post_generation(build_root, conf_dirs)

        # Compute group bases here
        bases = compute_bases(self)

        for dg in self.deploy_groups.values():
            if dg.sizes is None:
                continue
            dg.memories = Memories((bases[dg][0], 1024 ** 3), (bases[dg][1], 1024 ** 3))
            dg.group.generate_group_dir(build_root)

        for g in self.groups:
            g.post_generation2(build_root, conf_dirs)


def compute_bases(node: DeployNode):
    res = {}
    cur_bases = (node.node.memories.rom[0], node.node.memories.ram[0])
    for dg in node.deploy_groups.values():
        if dg.sizes is None:
            continue
        print(cur_bases, dg.sizes)
        res[dg] = cur_bases
        cur_bases = tuple(sum(x) for x in zip(cur_bases, dg.sizes))
        print(cur_bases)
    return res


class Deployment:
    nodes: [DeployNode]

    def __init__(self, nodes: [DeployNode]):
        self.nodes = nodes

    def _create_root_cmake(self, build_at):
        template = env.get_template("build_dir/CMakeLists.txt")
        with open(os.path.join(build_at, "CMakeLists.txt"), mode="w+") as root_cmake:
            with open(os.path.join(build_at, "tos/CMakeLists.txt")) as tos_cmake:
                root_cmake.write(template.render({
                    "tos_cmake_contents": tos_cmake.read(),
                    "group_subdirs": (group.name for node in self.nodes for group in node.deploy_groups),
                    "node_subdirs": (node.node.name for node in self.nodes)
                }))

    def _unique_platforms(self):
        return {node.node.platform for node in self.nodes}

    def _create_configurations(self, build_at):
        return {p: p.generateBuildDirectories(build_at) for p in self._unique_platforms()}

    def generate_build_dir(self, build_root):
        self._create_root_cmake(build_root)

        for node in self.nodes:
            node.generate_node_dir(build_root)

        # We need to build user groups first.
        # To do so, we need to create configurations for each unique user_cpu and compute their sizes first
        # Then, we'll build the final groups
        self.conf_dirs = self._create_configurations(build_root)
        print(self.conf_dirs)

        for node in self.nodes:
            node.post_generate(build_root, self.conf_dirs)

        for node in self.nodes:
            node.generate_node_dir(build_root)

    def build_all(self):
        for node in self.nodes:
            conf_dir = self.conf_dirs[node.node.platform][-1]
            args = ["ninja", f"{node.target_name}"]
            print(args)
            cmake_proc = subprocess.Popen(args, cwd=conf_dir)
            cmake_proc.wait()
