from .defs import *
from .imported_service import ImportedService
from .service_instance import ServiceInstance


def make_exports(group: DeployGroup):
    src_template = env.get_template("node/loaders/export_common.cpp")

    exported_servs = list(serv for serv in group.group.servs if len(serv.exports) > 0)
    export_strings = (export.cxx_export_string() for serv in exported_servs for export in serv.exports.values())
    export_includes = (exporter.get_cxx_include() for exporter in
                       set(export.exporter for serv in exported_servs for export in serv.exports.values()))

    return src_template.render({
        "group_name": group.group.name,
        "export_strings": export_strings,
    })


class BundledElfLoader(GroupLoader):
    user_src: str = None

    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/elf_group_loader/loader.cpp")
        header_template = env.get_template("node/loaders/elf_group_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/elf_group_loader/CMakeLists.txt")

        return {
            f"{group.group.name}.hpp": header_template.render({
                "group_name": group.group.name,
                "service_includes": group.group.cxx_ordered_includes(),
                "services": {serv.name: serv.registry_type() for serv in group.group.servs},
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()}
            }),
            "loader.cpp": src_template.render({
                "group_name": group.group.name,
                "service_includes": group.group.cxx_ordered_includes(),
                "service_types": (serv.registry_type() for serv in group.group.servs),
                "service_names": (serv.name for serv in group.group.servs),
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()},
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": group.group.cmake_targets(),
                "group_build_dir": self.user_src
            }),
            "exports.cpp": make_exports(group)
        }


class InMemoryLoader(GroupLoader):
    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/in_memory_loader/loader.cpp")
        header_template = env.get_template("node/loaders/in_memory_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/in_memory_loader/CMakeLists.txt")

        return {
            f"{group.group.name}.hpp": header_template.render({
                "group_name": group.group.name,
                "service_includes": group.group.cxx_ordered_includes(),
                "services": {serv.name: serv.impl.server_name() for serv in group.group.servs},
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()}
            }),
            "loader.cpp": src_template.render({
                "group_name": group.group.name,
                "service_includes": group.group.cxx_ordered_includes(),
                "service_types": (serv.registry_type() for serv in group.group.servs),
                "service_names": (serv.name for serv in group.group.servs),
                "imported_services": {key.name: val - 1 for key, val in group.group.assignNumsToExternalDeps()},
                "start_addr": hex(group.entry_point)
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": group.group.cmake_targets(),
            }),
            "exports.cpp": make_exports(group)
        }


class KernelLoader(GroupLoader):
    def generateGroupLoader(self, group: DeployGroup):
        src_template = env.get_template("node/loaders/in_kernel_loader/loader.cpp")
        header_template = env.get_template("node/loaders/in_kernel_loader/group.hpp")
        cmake_template = env.get_template("node/loaders/in_kernel_loader/CMakeLists.txt")

        needs_init = (serv for serv in group.group.servs if serv.needs_init())

        init_includes = []
        initializers = []
        for serv in needs_init:
            if isinstance(serv, ServiceInstance):
                dep_args = {}
                for name, dep in serv.deps.items():
                    dep_args[name] = f"co_await registry.wait<\"{dep.name}\">()"

                    # Detect if all dependency sync-ness match here
                    # If not, emit a bridge
                    if serv.is_async() == dep.is_async():
                        continue

                    print(
                        f"{serv.name}: Color mismatch for dependency {dep.name} ({serv.is_async()} -> {dep.is_async()})")
                    init_includes.append("lidlrt/transport/sync_to_async.hpp")
                    if serv.is_async():
                        dep_args[
                            name] = f"new {dep.get_interface().async_zerocopy_client()}<lidl::sync_to_async_bridge>({dep_args[name]})"
                    else:
                        raise NotImplementedError("Async to sync bridge not implemented!")

                dep_args = dep_args.values()
                init_str = f"init_{serv.impl.name}({', '.join(dep_args)})"
                if serv.is_async():
                    init_str = f"co_await {init_str}"
                init_str = f"registry.register_service<\"{serv.name}\">({init_str});"
                initializers.append(init_str)
            elif isinstance(serv, ImportedService):
                init_str = serv._import.cxx_import_string()
                init_str = f"registry.register_service<\"{serv.name}\">({init_str});"
                initializers.append(init_str)
                init_includes.extend(serv._import.cxx_includes())
            else:
                raise NotImplementedError("Don't know how to load this type in the kernel")

        return {
            f"{group.group.name}.hpp": header_template.render({
                "group_name": group.group.name,
                "service_includes": group.group.cxx_ordered_includes(),
                "services": {serv.name: serv.registry_type() for serv in group.group.servs},
            }),
            "loader.cpp": src_template.render({
                "group_name": group.group.name,
                "services": (serv.registry_type() for serv in group.group.servs),
                "service_init_sigs": group.group.generateInitSigSection(),
                "service_inits": initializers,
                "init_includes": init_includes,
            }),
            "CMakeLists.txt": cmake_template.render({
                "node_name": group.node.node.name,
                "group_name": group.group.name,
                "schemas": group.group.cmake_targets(),
                "service_targets": (serv.impl.cmake_target for serv in group.group.servs if
                                    isinstance(serv, ServiceInstance))
            }),
            "exports.cpp": make_exports(group)
        }
