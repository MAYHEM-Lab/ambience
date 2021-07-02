from .defs import *
from .groups.user_group import *
from .groups.kernel_group import *
from .platforms import *


def sample_deployment() -> [DeployNode]:
    calculator_mod = LidlModule("/home/tos/ambience/services/interfaces/calc.lidl", "calc_schema")
    logger_mod = LidlModule("/home/tos/src/core/log.yaml", "log_schema")
    alarm_mod = LidlModule("/home/tos/ambience/services/interfaces/alarm.lidl", "alarm_schema")
    fs_mod = LidlModule("/home/tos/ambience/services/interfaces/file_system.lidl", "filesystem_schema")
    block_mem_mod = LidlModule("/home/tos/ambience/services/interfaces/block_memory.lidl", "block_memory_schema")
    db_mod = LidlModule("/home/tos/ambience/services/interfaces/database.lidl", "database_schema")

    calc_if = calculator_mod.get_service("tos::ae::services::calculator")
    logger_if = logger_mod.get_service("tos::services::logger")
    alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
    fs_if = fs_mod.get_service("tos::ae::services::filesystem")
    block_mem_if = block_mem_mod.get_service("tos::ae::services::block_memory")
    db_if = fs_mod.get_service("tos::ae::services::sql_database")

    littlefs = fs_if.implement("littlefs_server", cmake_target="littlefs_server", sync=True, extern=False,
                               deps={"block": block_mem_if})

    basic_calc = calc_if.implement("basic_calc", cmake_target="basic_calc", sync=False, extern=False,
                                   deps={"logger": logger_if, "alarm": alarm_if, "fs": fs_if})

    virtio_block = block_mem_if.implement("virtio_blk", sync=True, extern=True)
    logger_impl = logger_if.implement("logger", sync=True, extern=True)
    alarm_impl = alarm_if.implement("alarm", sync=False, extern=True)

    virtio_blk = virtio_block.instantiate("node_block")
    logger = logger_impl.instantiate("logger")
    alarm = alarm_impl.instantiate("alarm")

    fs = littlefs.instantiate("fs", deps={"block": virtio_blk})
    calc = basic_calc.instantiate("calc", deps={"logger": logger, "alarm": alarm, "fs": fs})
    calc2 = basic_calc.instantiate("calc2", deps={"logger": logger, "alarm": alarm, "fs": fs})

    pg = KernelGroup("vm_privileged", {logger, alarm, virtio_blk, fs})
    g1 = UserGroup("sample_group3", {calc})
    g2 = UserGroup("sample_group4", {calc2})

    vm = Node("vm", x86_64,
              Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)))

    all_nodes = [
        vm.platform.make_deploy_node(vm, [pg, g1, g2]),
    ]

    return Deployment(all_nodes)
