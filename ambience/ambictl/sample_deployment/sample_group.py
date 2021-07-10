from ambictl import *
from .platforms import *

def x86_64_pc_node(name: str):
    return x86_64_pc.make_node(name,
                               Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                               exporters=[LwipUdpExporter()],
                               importers=[LwipUdpImporter()])


def digitalocean_vm_node(name: str):
    return digitalocean_vm.make_node(name,
                                     Memories((0x8000000 + 128 * 1024, 256 * 1024),
                                              (0x20000000 + 64 * 1024, 64 * 1024)),
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
                           exporters=[], importers=[XbeeImporter("tos::stm32::usart*")])


def hosted_node(name: str):
    return x86_hosted.make_node(name,
                                Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)),
                                exporters=[HostedUdpExporter(), XbeeExporter("tos::hosted::usart*")],
                                importers=[])


def sample_deployment() -> [DeployNode]:
    calculator_mod = LidlModule("//ambience/services/interfaces/calc.lidl", "calc_schema")
    alarm_mod = LidlModule("//ambience/services/interfaces/alarm.lidl", "alarm_schema")
    fs_mod = LidlModule("//ambience/services/interfaces/file_system.lidl", "filesystem_schema")
    block_mem_mod = LidlModule("//ambience/services/interfaces/block_memory.lidl", "block_memory_schema")
    machine_mod = LidlModule("//ambience/services/interfaces/machine.lidl", "machine_schema")
    echo_mod = LidlModule("//ambience/services/interfaces/echo.lidl", "echo_schema")
    db_mod = LidlModule("//ambience/services/interfaces/database.lidl", "database_schema")
    logger_mod = LidlModule("//src/core/log.yaml", "log_schema")

    calc_if = calculator_mod.get_service("tos::ae::services::calculator")
    logger_if = logger_mod.get_service("tos::services::logger")
    alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
    fs_if = fs_mod.get_service("tos::ae::services::filesystem")
    machine_if = machine_mod.get_service("tos::ae::services::machine")
    block_mem_if = block_mem_mod.get_service("tos::ae::services::block_memory")
    echo_if = echo_mod.get_service("tos::ae::services::echo")
    db_if = fs_mod.get_service("tos::ae::services::sql_database")

    basic_echo = echo_if.implement("basic_echo", cmake_target="basic_echo", sync=True,
                                   deps={"logger": logger_if})

    littlefs = fs_if.implement("littlefs_server", cmake_target="littlefs_server", sync=True,
                               deps={"block": block_mem_if})

    basic_calc = calc_if.implement("basic_calc", cmake_target="basic_calc", sync=False,
                                   deps={"logger": logger_if, "alarm": alarm_if, "fs": fs_if})

    hosted = hosted_node("hosted")
    vm = digitalocean_vm_node("vm")
    mcu = stm32l475("stm32")

    import_params = {
        "ip": "123.45.67.89",
        "port": 1993
    }

    serv = ImportedService("remote_fs", vm.importers[0].make_import(fs_if, import_params))

    virtio_blk = ExternService("node_block", block_mem_if, sync=True)
    fs_blk = ExternService("fs_block", block_mem_if, sync=True)
    logger = ExternService("logger", logger_if, sync=True)
    alarm = ExternService("alarm", alarm_if, sync=False)
    logger2 = ExternService("logger", logger_if, sync=True)
    alarm2 = ExternService("alarm", alarm_if, sync=False)
    logger3 = ExternService("logger", logger_if, sync=True)
    alarm3 = ExternService("alarm", alarm_if, sync=False)
    machine = ExternService("machine", machine_if, sync=True)

    fs = ServiceInstance("fs", littlefs, deps={"block": fs_blk})
    calc = ServiceInstance("calc", basic_calc, deps={"logger": logger, "alarm": alarm, "fs": fs})
    calc2 = ServiceInstance("calc2", basic_calc, deps={"logger": logger, "alarm": alarm, "fs": fs})

    pg = KernelGroup("vm_privileged", {virtio_blk, fs_blk, logger, alarm, machine, fs})
    g1 = UserGroup("sample_group3", {calc})
    g2 = UserGroup("sample_group4", {calc2})

    hg = KernelGroup("hosted_group",
                     {logger2, alarm2, hosted.exporters[0], hosted.exporters[1],
                      ServiceInstance("basic_echo", basic_echo, deps={"logger": logger2})})

    sg = KernelGroup("stm32_group", {logger3, alarm3, mcu.importers[0], ImportedService("remote_echo", mcu.importers[0].make_import(echo_if, {"addr":"0x1234", "channel": 0}))})

    all_nodes = [
        vm.deploy([pg, g1, g2]),
        hosted.deploy([hg]),
        mcu.deploy([sg])
    ]

    for node in all_nodes:
        for exporter in node.node.exporters:
            for g in node.groups:
                for serv in g.servs:
                    serv.export(exporter)

    return Deployment(all_nodes)
