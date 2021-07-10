from .defs import DeployNode, env
import os
import subprocess


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
