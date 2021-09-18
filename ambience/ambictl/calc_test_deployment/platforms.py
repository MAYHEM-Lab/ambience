import subprocess
import os


class x86_64(Platform):
    board_name: str

    def __init__(self, board_name: str):
        super().__init__(BundledElfLoader())
        self.loader.user_src = "${CMAKE_SOURCE_DIR}/cmake-build-barex64-user"
        self.board_name = board_name

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

        return (user_conf_dir, conf_dir)

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
        if cmake_proc.wait() != 0:
            raise RuntimeError("Generation failed")

        return (user_conf_dir, conf_dir)


raspi3 = raspi3()


class stm32(Platform):
    def __init__(self):
        super().__init__(InMemoryLoader())

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
        if cmake_proc.wait() != 0:
            raise RuntimeError("Generation failed")

        return (user_conf_dir, conf_dir)


stm32 = stm32()

class nrf52840(Platform):
    def __init__(self):
        super().__init__(InMemoryLoader())

    def make_deploy_node(self, node: Node, groups: [Group]):
        res = DeployNode(node, groups)
        res.target_name = f"{node.name}_kernel"
        return res

    def generateBuildDirectories(self, source_dir: str):
        user_conf_dir = os.path.join(source_dir, f"cmake-build-nrf52840_dk-user")
        os.makedirs(user_conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD=nrf52840_dk", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=user_conf_dir)
        cmake_proc.wait()

        conf_dir = os.path.join(source_dir, f"cmake-build-nrf52840_dk")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD=nrf52840_dk", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=conf_dir)
        if cmake_proc.wait() != 0:
            raise RuntimeError("Generation failed")

        return (user_conf_dir, conf_dir)


nrf52840 = nrf52840()


class x86_hosted(Platform):
    def __init__(self):
        super().__init__(KernelLoader())

    def generateBuildDirectories(self, source_dir: str):
        conf_dir = os.path.join(source_dir, f"cmake-build-hosted")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_BOARD=hosted", "-DCMAKE_BUILD_TYPE=Release",
                "-DENABLE_LTO=OFF", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        env = os.environ.copy()
        env["CC"] = "/opt/llvm/bin/clang"
        env["CXX"] = "/opt/llvm/bin/clang++"
        cmake_proc = subprocess.Popen(args, cwd=conf_dir, env=env)
        if cmake_proc.wait() != 0:
            raise RuntimeError("Generation failed")
        return (conf_dir,)

    def make_deploy_node(self, node: Node, groups: [Group]):
        res = DeployNode(node, groups)
        res.target_name = f"{node.name}_kernel"
        return res


x86_hosted = x86_hosted()

platform(
    name="common",
    memories=Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
    node_services=[
        ExternService("logger", logger_if, sync=True),
        ExternService("alarm", alarm_if, sync=False),
        ExternService("node_block", block_mem_if, sync=True),
    ]
)

platform(
    name="x86_64",
    inherit="common",
    node_services=[
        ExternService("machine", machine_if, sync=True),
        ExternService("fs_block", block_mem_if, sync=True),
    ]
)

platform(
    name="x86_64_pc",
    inherit="x86_64",
    native=x86_64_pc,
    importers=[
        importer(
            network="udp-internet",
            native=LwipUdpImporter
        ),
    ]
)

platform(
    name="digitalocean_vm",
    inherit="x86_64",
    native=digitalocean_vm,
    importers=[
        importer(
            network="udp-internet",
            native=LwipUdpImporter
        ),
        importer(
            network="DO-SFO2",
            native=LwipUdpImporter
        ),
    ],
)

platform(
    name="nrf52840",
    memories=Memories((0x27000 + 128 * 1024, 256 * 1024), (0x20002ae8 + 64 * 1024, 64 * 1024)),
    native=nrf52840,
    importers=[
    ],
    node_services=[
        ExternService("logger", logger_if, sync=True),
        ExternService("alarm", alarm_if, sync=False),
    ]
)

platform(
    name="stm32l4",
    inherit="common",
    native=stm32,
    importers=[
        importer(
            network="xbee-home",
            native=lambda: XbeeImporter("tos::stm32::usart*")
        ),
    ]
)

platform(
    name="hosted",
    inherit="common",
    native=x86_hosted,
    importers=[
        importer(
            network="udp-internet",
            native=HostedUdpImporter
        ),
    ]
)
