import os
from jinja2 import Environment, PackageLoader, select_autoescape
import subprocess
from ambictl import defs
from ambictl import sample_group
import re
from elftools.elf.elffile import ELFFile
from elftools.elf.segments import Segment
from typing import List
import tempfile

env = Environment(
    loader=PackageLoader("ambictl"),
    autoescape=select_autoescape()
)


def create_build_root(build_at, tos_source: str):
    os.makedirs(build_at, exist_ok=True)

    if os.path.exists(os.path.join(build_at, "tos")):
        os.unlink(os.path.join(build_at, "tos"))

    os.symlink(tos_source, os.path.join(build_at, "tos"), target_is_directory=True)


def create_root_cmake(build_at, deploy_nodes: List[defs.DeployNode]):
    template = env.get_template("build_dir/CMakeLists.txt")
    with open(os.path.join(build_at, "CMakeLists.txt"), mode="w+") as root_cmake:
        with open(os.path.join(build_at, "tos/CMakeLists.txt")) as tos_cmake:
            root_cmake.write(template.render({
                "tos_cmake_contents": tos_cmake.read(),
                "group_subdirs": (group.name for node in deploy_nodes for group in node.deploy_groups),
                "node_subdirs": (node.node.name for node in deploy_nodes)
            }))


def unique_platforms(deploy_nodes: List[defs.DeployNode]):
    return {node.node.platform for node in deploy_nodes}


def create_configurations(build_at, deploy_nodes: List[defs.DeployNode]):
    return {p: p.generateBuildDirectories(build_at) for p in unique_platforms(deploy_nodes)}


def create_group_dir(build_root: str, group: defs.Group):
    subdir = os.path.join(build_root, group.name)
    os.makedirs(subdir, exist_ok=True)
    return subdir


def create_group_src(subdir: str, group):
    if isinstance(group, defs.Group):
        with open(os.path.join(subdir, "interface.cpp"), mode="w+") as src:
            src.write("")
        with open(os.path.join(subdir, "null.cpp"), mode="w+") as src:
            src.write("")
        with open(os.path.join(subdir, "linker.ld"), mode="w+") as src:
            src.write("")
        with open(os.path.join(subdir, "group.cpp"), mode="w+") as src:
            src.write(group.generateBody())
        with open(os.path.join(subdir, "CMakeLists.txt"), mode="w+") as src:
            src.write(group.generateCmake())
    if isinstance(group, defs.DeployGroup):
        if group.memories is None:
            with open(os.path.join(subdir, "interface.cpp"), mode="w+") as src:
                src.write(group.generateInterface())
        else:
            with open(os.path.join(subdir, "linker.ld"), mode="w+") as src:
                src.write(group.memories.generateLinker())


def extract_readelf(build_dir: str) -> str:
    cache_path = os.path.join(build_dir, "CMakeCache.txt")
    with open(cache_path) as cache:
        contents = cache.read()
        pattern = "TOOLCHAIN_READELF:FILEPATH=(.*?)\n"
        res = re.search(pattern, contents)
        return res[1]


def read_seg_sizes(binary_path: str):
    with open(binary_path, 'rb') as file:
        elffile = ELFFile(file)
        return elffile.get_segment(0).header.p_memsz, elffile.get_segment(1).header.p_memsz


def read_entry_point(binary_path: str):
    with open(binary_path, 'rb') as file:
        elffile = ELFFile(file)
        return elffile.header.e_entry


def compute_size(build_dir: str, group: defs.Group) -> (int, int):
    args = ["ninja", f"{group.name}_size"]
    print(args)
    cmake_proc = subprocess.Popen(args, cwd=build_dir)
    cmake_proc.wait()
    return read_seg_sizes(os.path.join(build_dir, "bin", f"{group.name}_size"))


def get_entry_point(build_dir: str, group: defs.Group) -> int:
    args = ["ninja", f"{group.name}"]
    print(args)
    cmake_proc = subprocess.Popen(args, cwd=build_dir)
    cmake_proc.wait()
    return read_entry_point(os.path.join(build_dir, "bin", f"{group.name}"))


def compute_bases(all_nodes: List[defs.DeployNode]):
    res = {}
    for node in all_nodes:
        cur_bases = (node.node.memories.rom[0], node.node.memories.ram[0])
        for dg in node.deploy_groups.values():
            print(cur_bases, dg.sizes)
            res[dg] = cur_bases
            cur_bases = tuple(sum(x) for x in zip(cur_bases, dg.sizes))
            print(cur_bases)
    return res


def create_node_dir(build_dir: str, node: defs.DeployNode):
    node_dir = os.path.join(build_dir, node.node.name)
    groups_dir = os.path.join(node_dir, "groups")
    os.makedirs(groups_dir, exist_ok=True)

    with open(os.path.join(node_dir, "CMakeLists.txt"), "w+") as node_cmake:
        template = env.get_template("node/CMakeLists.txt")
        node_cmake.write(template.render({
            "node_name": node.node.name,
            "node_groups": (g.name for g in node.deploy_groups.keys()),
            "schemas": set(iface.module.cmake_target for g in node.groups for iface in g.interfaceDeps())
        }))

    with open(os.path.join(groups_dir, "CMakeLists.txt"), "w+") as groups_cmake:
        template = env.get_template("node/groups/CMakeLists.txt")
        groups_cmake.write(template.render({
            "node_groups": (g.name for g in node.deploy_groups.keys())
        }))

    with open(os.path.join(node_dir, "groups.hpp"), "w+") as node_src:
        template = env.get_template("node/groups.hpp")
        node_src.write(template.render({
            "groups": (group.name for group in node.deploy_groups),
            "group_includes": (f"{group.name}.hpp" for group in node.deploy_groups)
        }))

    with open(os.path.join(node_dir, "groups.cpp"), "w+") as node_src:
        template = env.get_template("node/groups.cpp")
        node_src.write(template.render({
            "groups": list(group.name for group in node.deploy_groups)
        }))

    with open(os.path.join(node_dir, "registry.hpp"), "w+") as node_src:
        node_src.write(node.generateRegistry())

    for dg in node.deploy_groups.values():
        group_dir = os.path.join(groups_dir, dg.group.name)
        os.makedirs(group_dir, exist_ok=True)
        loader = dg.generateGroupLoader()
        for name, content in loader.items():
            with open(os.path.join(group_dir, name), "w+") as group_src:
                group_src.write(content)

    return node_dir


if __name__ == "__main__":
    build_root = tempfile.mkdtemp()
    create_build_root(build_root, "/home/fatih/tos")

    all_nodes = sample_group.sample_deployment()

    for node in all_nodes:
        create_node_dir(build_root, node)

    for node in all_nodes:
        for g, dg in node.deploy_groups.items():
            dg.source_dir = create_group_dir(build_root, g)
            create_group_src(dg.source_dir, g)
            create_group_src(dg.source_dir, dg)

    create_root_cmake(build_root, all_nodes)

    # We need to build user groups first.
    # To do so, we need to create configurations for each unique user_cpu and compute their sizes first
    # Then, we'll build the final groups
    conf_dirs = create_configurations(build_root, all_nodes)
    print(conf_dirs)

    for i in range(len(all_nodes)):
        for g, dg in all_nodes[i].deploy_groups.items():
            dg.sizes = compute_size(conf_dirs[all_nodes[i].node.platform][0], g)
            print(dg.sizes)

    # Compute group bases here
    bases = compute_bases(all_nodes)

    for node in all_nodes:
        for dg in node.deploy_groups.values():
            dg.memories = defs.Memories((bases[dg][0], 1024 ** 3), (bases[dg][1], 1024 ** 3))
            create_group_src(dg.source_dir, dg)

    for i in range(len(all_nodes)):
        for g, dg in all_nodes[i].deploy_groups.items():
            dg.entry_point = get_entry_point(conf_dirs[all_nodes[i].node.platform][0], g)
            print(dg.entry_point)

    for node in all_nodes:
        create_node_dir(build_root, node)

    for node in all_nodes:
        conf_dir = conf_dirs[node.node.platform][-1]
        args = ["ninja", f"{node.target_name}"]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=conf_dir)
        cmake_proc.wait()
