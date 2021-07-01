from __future__ import annotations

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

    def get_include(self):
        return os.path.splitext(self.module.file_name())[0] + "_generated.hpp"

    def implement(self, name: str, *, sync: bool, extern: bool,
                  deps: Dict[str, ServiceInterface] = {}, cmake_target: str = "") -> Service:
        return Service(name, cmake_target, self, sync, extern, deps)


class Service:
    name: str
    cmake_target: str
    iface: ServiceInterface

    # Whether a service is async/sync is only important if both the client and the server is on the same group.
    # For instance, if a sync service is running in user space but the client is in the kernel, the kernel will still
    # see an async interface.
    sync: bool

    deps: Dict[str, ServiceInterface]

    # An extern service is one that has non-ambience dependencies, and cannot be initialized only with ambience services
    # For instance, a hardware driver service.
    extern: bool

    def __init__(self, name: str, cmake_target: str, iface: ServiceInterface, sync: bool, extern: bool,
                 deps: Dict[str, ServiceInterface] = {}) -> None:
        self.name = name
        self.cmake_target = cmake_target
        self.iface = iface
        self.deps = deps
        self.sync = sync
        self.extern = extern

    def getInitSignature(self) -> str:
        assert not self.extern
        params = (f"{val.sync_server_name() if self.sync else val.async_server_name()}* {key}" for key, val in
                  self.deps.items())
        if self.sync:
            return f"auto init_{self.name}({', '.join(params)}) -> {self.server_name()}*;"
        else:
            return f"auto init_{self.name}({', '.join(params)}) -> tos::Task<{self.server_name()}*>;"

    def instantiate(self, name: str, deps: Dict[str, ServiceInstance] = {}) -> ServiceInstance:
        return ServiceInstance(name, self, deps)

    def server_name(self):
        if self.sync:
            return self.iface.sync_server_name()
        else:
            return self.iface.async_server_name()

class ServiceInstance:
    name: str
    impl: Service
    deps: Dict[str, ServiceInstance]

    assigned_group: Group

    def unmetDeps(self):
        return [nm for nm, dep in self.deps.items() if dep is None]

    def __init__(self, name: str, impl: Service, deps: Dict[str, ServiceInstance] = {}) -> None:
        self.name = name
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

class Group:
    name: str
    servs: Set[ServiceInstance]
    dg: DeployGroup

    def __init__(self, name: str, servs: Set[ServiceInstance]) -> None:
        self.name = name
        self.servs = servs
        for serv in self.servs:
            serv.assigned_group = self

    def servsWithUnmetDeps(self):
        return [serv for serv in self.servs if len(serv.unmetDeps()) != 0]

    def uniqueDeps(self) -> Set[ServiceInstance]:
        return set(dep for serv in self.servs for _, dep in serv.deps.items())

    def uniqueExternalDeps(self):
        return set(dep for dep in self.uniqueDeps() if dep not in self.servs)

    def interfaceDeps(self):
        all_ifaces = {serv.impl.iface: set(serv.impl.deps.values()) for serv in self.servs}
        return toposort_flatten(all_ifaces, sort=False)

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

class Node:
    name: str
    platform: Platform
    memories: Memories

    def __init__(self, name: str, platform: Platform, memories: Memories):
        self.name = name
        self.platform = platform
        self.memories = memories


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
