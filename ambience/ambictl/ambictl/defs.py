from __future__ import annotations
from typing import Dict, List, Set
from toposort import toposort, toposort_flatten
from jinja2 import Environment, PackageLoader, select_autoescape
import os.path
import math

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

    def implement(self, name: str, *, sync: bool, extern: bool,
                  deps: Dict[str, ServiceInterface] = {}, cmake_target: str = "") -> Service:
        return Service(name, cmake_target, self, sync, extern, deps)

    def get_include(self):
        return os.path.splitext(self.module.file_name())[0] + "_generated.hpp"


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
        if not self.assigned_group.privileged:
            return self.impl.iface.async_server_name()

        return self.impl.server_name()


class Group:
    name: str
    servs: Set[ServiceInstance]
    privileged: bool

    def __init__(self, name: str, servs: Set[ServiceInstance], *, privileged: bool = False) -> None:
        self.name = name
        self.servs = servs
        self.privileged = privileged
        for serv in self.servs:
            serv.assigned_group = self

    def servsWithUnmetDeps(self):
        return [serv for serv in self.servs if len(serv.unmetDeps()) != 0]

    def uniqueDeps(self) -> Set[ServiceInstance]:
        return set(dep for serv in self.servs for _, dep in serv.deps.items())

    def uniqueExternalDeps(self):
        return set(dep for dep in self.uniqueDeps() if dep not in self.servs)

    def assignNumsToExternalDeps(self) -> Dict[ServiceInstance, int]:
        if len(self.servsWithUnmetDeps()) != 0:
            raise "Not all dependencies are met!"
        return list((y, x + 1) for x, y in enumerate(self.uniqueExternalDeps()))

    def generateInitSigSection(self):
        unique_services = list(set(s.impl for s in self.servs))

        serv_initializers = (s.getInitSignature() for s in unique_services)

        return "\n".join(serv_initializers)

    def generateExternalDepsSection(self):
        numbering = self.assignNumsToExternalDeps()
        return "\n".join(
            f"auto ext_dep{num} = transport.get_service<{serv.impl.iface.absolute_name()}, {num}>();" for serv, num in
            numbering)

    def _generate_init_section(self):
        ext_deps = self.uniqueExternalDeps()
        serv_name_mapping = {}
        for serv, num in self.assignNumsToExternalDeps():
            serv_name_mapping[serv] = f"ext_dep{num}"

        for serv in self.servs:
            serv_name_mapping[serv] = serv.name

        dep_dict = {serv: set(serv for serv in serv.deps.values() if serv not in ext_deps) for serv in self.servs}

        res = []
        for s in toposort(dep_dict):
            for serv in s:
                args = (serv_name_mapping[dep] for dep in serv.deps.values())
                res.append(f"auto {serv.name} = co_await init_{serv.impl.name}({', '.join(args)});")

        return res

    def interfaceDeps(self):
        all_ifaces = {serv.impl.iface: set(serv.impl.deps.values()) for serv in self.servs}
        return toposort_flatten(all_ifaces, sort=False)

    def generateBody(self):
        template = env.get_template("group/group.cpp")
        return template.render({
            "group": self,
            "service_init_sigs": self.generateInitSigSection(),
            "external_deps": self.generateExternalDepsSection(),
            "service_inits": self._generate_init_section(),
            "group_init": f"::g = new tos::ae::group<{len(self.servs)}>(tos::ae::group<{len(self.servs)}>::make({', '.join(serv.name for serv in self.servs)}));",
            "service_includes": (iface.get_include() for iface in self.interfaceDeps())
        })

    def generateCmake(self):
        template = env.get_template("group/CMakeLists.txt")
        return template.render({
            "group_name": self.name,
            "schemas": (iface.module.cmake_target for iface in self.interfaceDeps()),
            "service_targets": (serv.impl.cmake_target for serv in self.servs)
        })


class Memories:
    rom: (int, int)
    ram: (int, int)

    def __init__(self, rom, ram):
        self.rom = rom
        self.ram = ram

    def generateLinker(self):
        template = env.get_template("group/linker.ld")
        return template.render({
            "rom_base": hex(self.rom[0]),
            "rom_size": f"{math.ceil(self.rom[1] / 1024)}K",
            "ram_base": hex(self.ram[0]),
            "ram_size": f"{math.ceil(self.ram[1] / 1024)}K"
        })


class DeployGroup:
    node: DeployNode
    group: Group
    heap_size: int
    queue_size: int
    sizes: (int, int)
    memories: Memories
    source_dir: str
    kernel_dir: str

    def __init__(self, node: DeployNode, group: Group, heap_size: int, queue_size: int):
        self.node = node
        self.group = group
        self.heap_size = heap_size
        self.queue_size = queue_size
        self.memories = None
        self.sizes = None
        self.source_dir = None
        self.kernel_dir = None

    def generateInterface(self):
        template = env.get_template("group/interface.cpp")
        return template.render({
            "queue_size": self.queue_size,
        })

    def generateGroupLoader(self):
        return self.node.node.platform.generateGroupLoader(self)


class Platform:
    tos_cpu: str
    user_cpu: str
    loader: GroupLoader

    def __init__(self, cpu: str, user_cpu: str, loader: GroupLoader):
        self.tos_cpu = cpu
        self.user_cpu = user_cpu
        self.loader = loader

    def generateGroupLoader(self, group: DeployGroup):
        return self.loader.generateGroupLoader(group)


class GroupLoader:
    def generateGroupLoader(self, group: DeployGroup):
        raise NotImplementedError()


class BundledElfLoader(GroupLoader):
    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/groups/elf_group_loader/loader.cpp")
        header_template = env.get_template("node/groups/elf_group_loader/group.hpp")
        cmake_template = env.get_template("node/groups/elf_group_loader/CMakeLists.txt")

        return {
            f"{group.group.name}.hpp": header_template.render({
                "group_name": group.group.name,
                "service_includes": (iface.get_include() for iface in group.group.interfaceDeps()),
                "services": {serv.name: serv.impl.server_name() for serv in group.group.servs},
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()}
            }),
            "loader.cpp": src_template.render({
                "group_name": group.group.name,
                "service_includes": (iface.get_include() for iface in group.group.interfaceDeps()),
                "services": {serv.name: serv.impl.server_name() for serv in group.group.servs},
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()}
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": (iface.module.cmake_target for iface in group.group.interfaceDeps()),
                "group_build_dir": "${CMAKE_SOURCE_DIR}/cmake-build-barex64-user"
            })
        }


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

    def __init__(self, node: Node, groups: List[Group]):
        self.node = node
        self.groups = groups
        self.deploy_groups = {g: DeployGroup(self, g, 2048, 32) for g in self.groups if not g.privileged}

    def visibleServices(self):
        return (serv for group in self.groups for serv in group.servs)

    def generateRegistry(self):
        template = env.get_template("node/registry.hpp")
        return template.render({
            "services": {serv.name: serv.registry_type() for serv in self.visibleServices()},
            "service_includes": set(iface.get_include() for group in self.groups for iface in group.interfaceDeps())
        })
