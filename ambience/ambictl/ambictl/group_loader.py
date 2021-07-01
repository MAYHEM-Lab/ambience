import ambictl.deploy_group


class GroupLoader:
    def generateGroupLoader(self, group: DeployGroup):
        raise NotImplementedError()


class BundledElfLoader(GroupLoader):
    user_src: str = None

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
                "services": (serv.impl.server_name() for serv in group.group.servs),
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
        src_template = env.get_template("node/groups/in_memory_loader/loader.cpp")
        header_template = env.get_template("node/groups/in_memory_loader/group.hpp")
        cmake_template = env.get_template("node/groups/in_memory_loader/CMakeLists.txt")

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
