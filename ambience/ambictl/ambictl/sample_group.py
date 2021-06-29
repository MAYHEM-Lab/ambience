from .defs import LidlModule, Group, Platform, BundledElfLoader, Node, Memories, DeployNode, InMemoryLoader
import os
import subprocess


class x86_64(Platform):
    def __init__(self):
        super().__init__(BundledElfLoader())
        self.loader.user_src = "${CMAKE_SOURCE_DIR}/cmake-build-barex64-user"
        pass

    def generateBuildDirectories(self, source_dir: str):
        user_conf_dir = os.path.join(source_dir, f"cmake-build-barex64-user")
        os.makedirs(user_conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=x86_64/ae_user", "-DCMAKE_BUILD_TYPE=MinSizeRel",
                "-DENABLE_LTO=ON", "-DCMAKE_CXX_COMPILER_LAUNCHER=ccache", "-DCMAKE_C_COMPILER_LAUNCHER=ccache",
                source_dir]
        print(args)
        cmake_proc = subprocess.Popen(args, cwd=user_conf_dir)
        cmake_proc.wait()

        conf_dir = os.path.join(source_dir, f"cmake-build-barex64")
        os.makedirs(conf_dir, exist_ok=True)
        args = ["cmake", "-G", "Ninja", f"-DTOS_CPU=x86_64/bare", "-DCMAKE_BUILD_TYPE=MinSizeRel",
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


x86_64 = x86_64()


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

def sample_deployment() -> [DeployNode]:
    calculator_mod = LidlModule("/home/tos/ambience/services/interfaces/calc.lidl", "calc_schema")
    logger_mod = LidlModule("/home/tos/src/core/log.yaml", "log_schema")
    alarm_mod = LidlModule("/home/tos/ambience/services/interfaces/alarm.lidl", "alarm_schema")
    fs_mod = LidlModule("/home/tos/ambience/services/interfaces/file_system.lidl", "filesystem_schema")
    db_mod = LidlModule("/home/tos/ambience/services/interfaces/database.lidl", "database_schema")

    calc_if = calculator_mod.get_service("tos::ae::services::calculator")
    logger_if = logger_mod.get_service("tos::services::logger")
    alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
    fs_if = fs_mod.get_service("tos::ae::services::filesystem")
    db_if = fs_mod.get_service("tos::ae::services::sql_database")

    basic_calc = calc_if.implement("basic_calc", cmake_target="basic_calc", sync=False, extern=False,
                                   deps={"logger": logger_if, "alarm": alarm_if, "fs": fs_if})

    logger_impl = logger_if.implement("logger", sync=True, extern=True)
    alarm_impl = alarm_if.implement("alarm", sync=False, extern=True)
    fs_impl = fs_if.implement("fs", sync=True, extern=True)
    sqlite_impl = db_if.implement("sqlite3", sync=True, extern=True)

    logger = logger_impl.instantiate("logger")
    alarm = alarm_impl.instantiate("alarm")
    fs = fs_impl.instantiate("fs")
    sqlite = fs_impl.instantiate("sqlite")

    calc = basic_calc.instantiate("calc", deps={"logger": logger, "alarm": alarm, "fs": fs})

    pg = Group("vm_privileged", {logger, alarm, fs}, privileged=True)
    g1 = Group("sample_group3", {calc})

    db_pg = Group("cloud_privileged", {sqlite}, privileged=True)

    vm = Node("vm", stm32,
              Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)))

    all_nodes = [
        vm.platform.make_deploy_node(vm, [pg, g1]),
    ]

    return all_nodes
