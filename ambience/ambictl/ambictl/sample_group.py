from .defs import *
from .groups.user_group import *
from .groups.kernel_group import *
from .platforms import *


def x86_64_pc(name: str):
    return x86_64.make_node(name,
                            Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                            exporters=[LwipUdpExporter()],
                            importers=[LwipUdpImporter()])


def raspi3(name: str):
    return raspi3.make_node(name,
                            Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                            exporters=[LwipUdpExporter()],
                            importers=[LwipUdpImporter()])


def stm32l475(name: str):
    return stm32.make_node(name,
                           Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                           exporters=[], importers=[])


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

    littlefs = fs_if.implement("littlefs_server", cmake_target="littlefs_server", sync=True,
                               deps={"block": block_mem_if})

    basic_calc = calc_if.implement("basic_calc", cmake_target="basic_calc", sync=False,
                                   deps={"logger": logger_if, "alarm": alarm_if, "fs": fs_if})

    vm = x86_64_pc("vm")

    import_params = {
        "ip": "123.45.67.89",
        "port": 1993
    }

    serv = vm.importers[0].make_import(fs_if, import_params).instantiate("remote_fs")

    virtio_blk = ExternService("node_block", block_mem_if, sync=True)
    logger = ExternService("logger", logger_if, sync=True)
    alarm = ExternService("alarm", alarm_if, sync=False)

    fs = littlefs.instantiate("fs", deps={"block": virtio_blk})
    calc = basic_calc.instantiate("calc", deps={"logger": logger, "alarm": alarm, "fs": fs})
    calc2 = basic_calc.instantiate("calc2", deps={"logger": logger, "alarm": alarm, "fs": serv})

    pg = KernelGroup("vm_privileged", {virtio_blk, logger, alarm, serv, fs})
    g1 = UserGroup("sample_group3", {calc})
    g2 = UserGroup("sample_group4", {calc2})

    all_nodes = [
        vm.deploy([pg, g1, g2]),
    ]

    for node in all_nodes:
        for exporter in node.node.exporters:
            for g in node.groups:
                for serv in g.servs:
                    serv.export(exporter)

    return Deployment(all_nodes)
