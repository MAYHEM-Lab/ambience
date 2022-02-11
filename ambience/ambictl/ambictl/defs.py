from __future__ import annotations

import collections
import enum
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


def _write_if_different(file, content):
    # Check if content is the same; if so, exit early
    if os.path.exists(file):
        with open(file, 'r') as f:
            if f.read() == content:
                return
    
    # Otherwise write to file
    with open(file, 'w+') as w:
        w.write(content)


class NodeObject:
    @abc.abstractmethod
    def cxx_type(self):
        pass

    @abc.abstractmethod
    def cxx_init_call(self):
        pass

    @abc.abstractmethod
    def cxx_include(self):
        pass

    @abc.abstractmethod
    def cmake_target(self):
        return ""


class LidlModule:
    _abs_path: str
    cmake_target: str
    deps: [LidlModule]

    def __init__(self, abs_path: str, cmake_target: str, deps: [LidlModule] = None):
        self._abs_path = abs_path
        self.cmake_target = cmake_target
        self.deps = [] if deps is None else deps

    def absolute_import(self):
        return self._abs_path

    def file_name(self):
        return os.path.basename(self._abs_path)

    def get_service(self, name: str) -> ServiceInterface:
        return ServiceInterface(self, name)

    def _cxx_include(self):
        return os.path.splitext(self.file_name())[0] + "_generated.hpp"

    def cxx_includes(self):
        res = {}

        for dep in self.deps:
            res = {**res, **dep.cxx_includes()}

        res[self._cxx_include()] = set(mod._cxx_include() for mod in self.deps)

        return res


import_export_mod = LidlModule("import_export.lidl", "import_export_schema")


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

    def async_stub_client(self):
        return f"{self.absolute_name()}::async_stub_client"

    def async_zerocopy_client(self):
        return f"{self.absolute_name()}::async_zerocopy_client"

    def cxx_interface_deps(self):
        return self.module.cxx_interface_deps()

    def implement(self, name: str, *, sync: bool,
                  deps: Dict[str, ServiceInterface] = {}, cmake_target: str = "") -> Service:
        return Service(name, cmake_target, self, sync, deps)

    def __deepcopy__(self, memo):
        return self


importer_if = import_export_mod.get_service("importer")
exporter_if = import_export_mod.get_service("exporter")


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

    def server_name(self):
        if self.sync:
            return self.iface.sync_server_name()
        else:
            return self.iface.async_server_name()

    def dependency_type(self, name):
        return self.deps[name]


class Instance:
    name: str
    exports: Dict[Exporter, Export]
    extern: bool
    assigned_group: Group

    @abc.abstractmethod
    def get_interface(self) -> ServiceInterface:
        raise NotImplementedError()

    def __init__(self, name: str):
        self.name = name
        self.exports = {}
        self.extern = False
        self.assigned_group = None

    def export(self, exporter, config=None):
        if exporter not in self.exports:
            print(f"Export {self.name} with {exporter}")
            self.exports[exporter] = exporter.export_service(self, config)
        return self.exports[exporter]

    def needs_init(self):
        return not self.extern

    @abc.abstractmethod
    def registry_type(self):
        raise NotImplementedError(f"registry_type not implemented in {self.__class__.__name__}")

    @abc.abstractmethod
    def is_async(self):
        raise NotImplementedError(f"is_async not implemented in {self.__class__.__name__}")

    @abc.abstractmethod
    def cxx_includes(self):
        raise NotImplementedError()

    @abc.abstractmethod
    def cmake_targets(self):
        return []


class Group:
    name: str
    servs: Set[Instance]
    dg: DeployGroup

    def __init__(self, name: str, servs: List[Instance]) -> None:
        self.name = name
        self.servs = servs
        for serv in self.servs:
            serv.assigned_group = self

    def add_service(self, ins: Instance):
        self.servs.append(ins)
        ins.assigned_group = self

    def _interfaceDeps(self):
        all_ifaces = {serv.get_interface(): set(serv.get_dependencies()) for serv in self.servs if
                      hasattr(serv, "get_dependencies")}
        for serv in self.servs:
            if serv.get_interface() not in all_ifaces:
                all_ifaces[serv.get_interface()] = set()
        return toposort_flatten(all_ifaces, sort=False)

    def cxx_ordered_includes(self):
        return [iface.module._cxx_include() for iface in self._interfaceDeps()]

    def cmake_targets(self):
        return (iface.module.cmake_target for iface in self._interfaceDeps())

    def generateInitSigSection(self):
        unique_impls = set(s.impl for s in self.servs if hasattr(s, "impl"))
        return "\n".join(s.cxx_init_signature() for s in unique_impls)

    @abc.abstractmethod
    def override_registry_type(self, ins):
        return None

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
    queue_size: int
    sizes: (int, int)
    memories: Memories
    entry_point: int

    def __init__(self, node: DeployNode, group: Group, queue_size: int):
        group.dg = self
        self.node = node
        self.group = group
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


class NetworkType(enum.Enum):
    UDP = 0
    TCP = 1
    XBee = 2
    UnixDomain = 3


class Network:
    name: str
    net_type: NetworkType
    importers: [Importer]  # These can get services from the network
    exporters: [Exporter]  # These can provide services to the network
    hop_cost: int

    def __init__(self, name: str, net_type: NetworkType, *, hop_cost=1000):
        self.name = name
        self.net_type = net_type
        self.importers = []
        self.exporters = []
        self.hop_cost = hop_cost

    def exporting_nodes(self):
        return set(exp.assigned_node for exp in self.exporters)

    def add_interface(self, importer_or_exporter):
        assert importer_or_exporter.net_type == self.net_type
        if isinstance(importer_or_exporter, Importer):
            self.importers.append(importer_or_exporter)
        elif isinstance(importer_or_exporter, Exporter):
            self.exporters.append(importer_or_exporter)
        else:
            raise RuntimeError("Interface must be an importer or exporter")
        importer_or_exporter.assigned_network = self

    def make_importer(self, importer_type):
        res = importer_type()
        assert res.net_type == self.net_type
        self.add_interface(res)
        return res

    def make_exporter(self, addr, exporter_type):
        res = exporter_type()
        assert res.net_type == self.net_type
        res.net_address = addr
        self.add_interface(res)
        return res

    def __repr__(self):
        return f"{self.name}({self.net_type.__repr__()})"


class Importer(Instance):
    net_type: NetworkType
    assigned_network: Network
    assigned_node: None
    hop_cost: int

    def __init__(self, name: str, net_type: NetworkType):
        super().__init__(name)
        self.extern = True
        self.net_type = net_type
        self.assigned_network = None
        self.assigned_node = None
        self.hop_cost = 0

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
    def is_async(self):
        return False

    def get_interface(self) -> ServiceInterface:
        return importer_if


class Exporter(Instance):
    net_type: NetworkType
    assigned_network: Network
    net_address: None
    assigned_node: Node
    hop_cost: int

    def __init__(self, name: str, net_type: NetworkType):
        super().__init__(name)
        self.extern = True
        self.net_type = net_type
        self.net_address = None
        self.assigned_network = None
        self.assigned_node = None
        self.hop_cost = 0

    @abc.abstractmethod
    def export_service(self, service: Instance, config) -> Export:
        pass

    @abc.abstractmethod
    def export_service_string(self, export: Export):
        pass

    def get_interface(self) -> ServiceInterface:
        return exporter_if


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

    def __repr__(self):
        return f"Export({repr(self.exporter)}, {repr(self.instance)}, {repr(self.config)})"


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

    def cxx_includes(self):
        return self.importer.cxx_includes()


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

    def make_node(self, name: str, mems: Memories, exporters: [Exporter] = None, importers: [Importer] = None):
        return Node(name, self, mems, [] if exporters is None else exporters, [] if importers is None else importers)


class Node:
    name: str
    platform: Platform
    memories: Memories
    exporters: [Exporter]
    importers: [Importer]
    node_services: [Instance]  # These services are exposed by the node itself
    deploy_node: DeployNode

    def __init__(self, name: str, platform: Platform, memories: Memories, exporters: [Exporter], importers: [Importer]):
        self.name = name
        self.platform = platform
        self.memories = memories
        self.exporters = exporters
        self.importers = importers
        self.node_services = []
        self.deploy_node = None

        for imp in self.importers:
            imp.assigned_node = self
            self.node_services.append(imp)

        for exp in self.exporters:
            exp.assigned_node = self
            self.node_services.append(exp)

    def deploy(self, groups: List[Group]):
        assert not self.deploy_node
        host_group = groups[0]
        for ns in self.node_services:
            if ns not in host_group.servs:
                host_group.add_service(ns)
        self.deploy_node = self.platform.make_deploy_node(self, groups)
        return self.deploy_node

    def __repr__(self):
        return self.name


class DeployNode:
    node: Node
    groups: List[Group]
    deploy_groups: Dict[Group, DeployGroup]
    target_name: str
    objects: {}

    def __init__(self, node: Node, groups: List[Group]):
        self.node = node
        self.groups = groups
        self.deploy_groups = {g: DeployGroup(self, g, queue_size=64) for g in self.groups}
        self.target_name = None
        self.objects = {}

    def visibleServices(self):
        return (serv for group in self.groups for serv in group.servs)

    def generateRegistry(self):
        template = env.get_template("node/registry.hpp")
        return template.render({
            "services": {serv.name: serv.registry_type() for serv in self.visibleServices()},
            "service_includes": (includes for group in self.groups for includes in group.cxx_ordered_includes()),
            "exporter_includes": set(
                (include for exporter in self.node.exporters for include in exporter.cxx_includes())).union(
                set((include for importer in self.node.importers for include in importer.cxx_includes()))),
        })

    def generate_node_dir(self, build_dir):
        for g in self.deploy_groups.keys():
            g.generate_group_dir(build_dir)

        node_dir = os.path.join(build_dir, self.node.name)
        loaders_dir = os.path.join(node_dir, "loaders")
        os.makedirs(loaders_dir, exist_ok=True)

        node_cmake = os.path.join(node_dir, "CMakeLists.txt")
        template = env.get_template("node/CMakeLists.txt")
        _write_if_different(node_cmake, template.render({
            "node_name": self.node.name,
            "node_groups": (g.name for g in self.deploy_groups.keys()),
            "schemas": set(target for g in self.groups for target in g.cmake_targets()),
            "link_targets": set(
                target for exporter in self.node.exporters for target in exporter.cmake_targets()).union(
                set(target for importer in self.node.importers for target in importer.cmake_targets()))
        }))

        groups_cmake = os.path.join(loaders_dir, "CMakeLists.txt")
        template = env.get_template("node/loaders/CMakeLists.txt")
        _write_if_different(groups_cmake, template.render({
            "node_loaders": (g.name for g in self.deploy_groups.keys())
        }))

        # node_src = os.path.join(node_dir, "groups.hpp")
        # template = env.get_template("node/groups.hpp")
        # _write_if_different(node_src, template.render({
        #     "groups": (group.name for group in self.deploy_groups),
        #     "group_includes": (f"{group.name}.hpp" for group in self.deploy_groups)
        # }))

        # node_src = os.path.join(node_dir, "groups.cpp")
        # template = env.get_template("node/groups.cpp")
        # _write_if_different(node_src, template.render({
        #     "groups": list(group.name for group in self.deploy_groups)
        # }))

        node_src = os.path.join(node_dir, "registry.hpp")
        _write_if_different(node_src, self.generateRegistry())

        for dg in self.deploy_groups.values():
            group_dir = os.path.join(loaders_dir, dg.group.name)
            os.makedirs(group_dir, exist_ok=True)
            loader = dg.group.generate_loader_dir("")
            for name, content in loader.items():
                group_src = os.path.join(group_dir, name)
                _write_if_different(group_src, content)

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
        print((hex(cur_bases[0]), hex(cur_bases[1])), dg.sizes)
        res[dg] = cur_bases
        cur_bases = tuple(sum(x) for x in zip(cur_bases, dg.sizes))
        print((hex(cur_bases[0]), hex(cur_bases[1])))
    return res
