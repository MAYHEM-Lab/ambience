import subprocess
import os
from .defs import *
from .group_loader import *

class x86_64(Platform):
    board_name: str
    def __init__(self, board_name: str):
        super().__init__(BundledElfLoader())
        self.loader.user_src = "${CMAKE_SOURCE_DIR}/cmake-build-barex64-user"
        self.board_name = board_name
        pass

    def generateBuildDirectories(self, source_dir: str):
        user_conf_dir = os.path.join(source_dir, f"cmake-build-barex64-user")
        os.makedirs(user_conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=x86_64/ae_user", "-DCMAKE_BUILD_TYPE=Release",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=user_conf_dir)
        cmake_proc.wait()

        conf_dir = os.path.join(source_dir, f"cmake-build-barex64")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD={self.board_name}", "-DCMAKE_BUILD_TYPE=Release",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=conf_dir)
        cmake_proc.wait()

        loader_dir = os.path.join(source_dir, f"cmake-build-barex86")
        os.makedirs(loader_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=x86/bare", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=loader_dir)
        cmake_proc.wait()

        return (user_conf_dir, conf_dir, loader_dir)

    def make_deploy_node(self, node: Node, groups: [Group]):
        res = DeployNode(node, groups)
        res.target_name = f"{node.name}-iso"
        return res


x86_64_pc = x86_64("x86_64_pc")
digitalocean_vm = x86_64("digitalocean_vm")


class raspi3(Platform):
    def __init__(self):
        super().__init__(BundledElfLoader())
        self.loader.user_src = "${CMAKE_SOURCE_DIR}/cmake-build-raspi3-user"
        pass

    def make_deploy_node(self, node: Node, groups: [Group]):
        res = DeployNode(node, groups)
        res.target_name = f"{node.name}_kernel"
        return res

    def generateBuildDirectories(self, source_dir: str):
        user_conf_dir = os.path.join(source_dir, f"cmake-build-raspi3-user")
        os.makedirs(user_conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=raspi/3", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=user_conf_dir)
        cmake_proc.wait()

        conf_dir = os.path.join(source_dir, f"cmake-build-raspi3")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=raspi/3", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=conf_dir)
        cmake_proc.wait()

        return (user_conf_dir, conf_dir)


raspi3 = raspi3()


class stm32(Platform):
    def __init__(self):
        super().__init__(InMemoryLoader())
        pass

    def make_deploy_node(self, node: Node, groups: [Group]):
        res = DeployNode(node, groups)
        res.target_name = f"{node.name}_kernel"
        return res

    def generateBuildDirectories(self, source_dir: str):
        user_conf_dir = os.path.join(source_dir, f"cmake-build-stm32l475-user")
        os.makedirs(user_conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD=iot_epd", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=user_conf_dir)
        cmake_proc.wait()

        conf_dir = os.path.join(source_dir, f"cmake-build-stm32l475")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD=iot_epd", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=conf_dir)
        cmake_proc.wait()

        return (user_conf_dir, conf_dir)


stm32 = stm32()
