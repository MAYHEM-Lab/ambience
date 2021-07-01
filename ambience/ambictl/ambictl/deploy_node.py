import os
import ambictl.group
import ambictl.deploy_group
import ambictl.node
from typing import Dict, List
import ambictl
import ambictl.memories


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
                "node_groups": (g.name for g in self.deploy_groups.keys())
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
            dg.memories = Memories((bases[dg][0], 1024 ** 3), (bases[dg][1], 1024 ** 3))
            dg.group.generate_group_dir(build_root)

        for g in self.groups:
            g.post_generation2(build_root, conf_dirs)

def compute_bases(node: DeployNode):
    res = {}
    cur_bases = (node.node.memories.rom[0], node.node.memories.ram[0])
    for dg in node.deploy_groups.values():
        print(cur_bases, dg.sizes)
        res[dg] = cur_bases
        cur_bases = tuple(sum(x) for x in zip(cur_bases, dg.sizes))
        print(cur_bases)
    return res
