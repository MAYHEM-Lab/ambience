from .defs import *


class BundledElfLoader(GroupLoader):
    user_src: str = None

    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/elf_group_loader/loader.cpp")
        header_template = env.get_template("node/loaders/elf_group_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/elf_group_loader/CMakeLists.txt")

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
                "service_types": (serv.impl.server_name() for serv in group.group.servs),
                "service_names": (serv.name for serv in group.group.servs),
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()}
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": (iface.module.cmake_target for iface in group.group.interfaceDeps()),
                "group_build_dir": self.user_src
            })
        }


class InMemoryLoader(GroupLoader):
    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/in_memory_loader/loader.cpp")
        header_template = env.get_template("node/loaders/in_memory_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/in_memory_loader/CMakeLists.txt")

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
                "services": (serv.impl.server_name() for serv in group.group.servs),
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()},
                "start_addr": hex(group.entry_point)
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": (iface.module.cmake_target for iface in group.group.interfaceDeps()),
            })
        }

class KernelLoader(GroupLoader):
    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/in_kernel_loader/loader.cpp")
        header_template = env.get_template("node/loaders/in_kernel_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/in_kernel_loader/CMakeLists.txt")

        needs_init = (serv for serv in group.group.servs if not serv.impl.extern)

        initializers = []
        for serv in needs_init:
            dep_args = (f"co_await registry.wait<\"{dep.name}\">()" for (name, dep) in serv.deps.items())
            init_str = f"init_{serv.impl.name}({', '.join(dep_args)})"
            if not serv.impl.sync:
                init_str = f"co_await {init_str}"
            init_str = f"registry.register_service<\"{serv.name}\">({init_str});"
            initializers.append(init_str)

        return {
            f"{group.group.name}.hpp": header_template.render({
                "group_name": group.group.name,
                "service_includes": (iface.get_include() for iface in group.group.interfaceDeps()),
                "services": {serv.name: serv.impl.server_name() for serv in group.group.servs},
            }),
            "loader.cpp": src_template.render({
                "group_name": group.group.name,
                "service_includes": (iface.get_include() for iface in group.group.interfaceDeps()),
                "services": (serv.impl.server_name() for serv in group.group.servs),
                "service_init_sigs": group.group.generateInitSigSection(),
                "service_inits": initializers,
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": (iface.module.cmake_target for iface in group.group.interfaceDeps()),
                "service_targets": (serv.impl.cmake_target for serv in group.group.servs if not serv.impl.extern)
            })
        }
