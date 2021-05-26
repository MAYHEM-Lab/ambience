from .defs import LidlModule, Group, Platform, BundledElfLoader, Node, Memories, DeployNode

x86_64 = Platform("x86_64/bare", BundledElfLoader())

vm = Node("vm", x86_64,
          Memories((0x8000000 + 128 * 1024, 256 * 1024), (0x20000000 + 64 * 1024, 64 * 1024)))


def sample_deployment() -> [DeployNode]:
    calculator_mod = LidlModule("/home/tos/ambience/services/interfaces/calc.lidl", "calc_schema")
    logger_mod = LidlModule("/home/tos/src/core/log.yaml", "log_schema")
    alarm_mod = LidlModule("/home/tos/ambience/services/interfaces/alarm.lidl", "alarm_schema")
    fs_mod = LidlModule("/home/tos/ambience/services/interfaces/file_system.lidl", "filesystem_schema")

    calc_if = calculator_mod.get_service("tos::ae::services::calculator")
    logger_if = logger_mod.get_service("tos::services::logger")
    alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
    fs_if = fs_mod.get_service("tos::ae::services::filesystem")

    basic_calc = calc_if.implement("basic_calc", cmake_target="basic_calc", sync=False, extern=False,
                                   deps={"logger": logger_if, "alarm": alarm_if, "fs": fs_if})

    logger_impl = logger_if.implement("logger", sync=True, extern=True)
    alarm_impl = alarm_if.implement("alarm", sync=False, extern=True)
    fs_impl = fs_if.implement("fs", sync=True, extern=True)

    logger = logger_impl.instantiate("logger")
    alarm = alarm_impl.instantiate("alarm")
    fs = fs_impl.instantiate("fs")

    calc = basic_calc.instantiate("calc", deps={"logger": logger, "alarm": alarm, "fs": fs})

    pg = Group("vm_privileged", {logger, alarm, fs}, privileged=True)
    g1 = Group("sample_group3", {calc})

    all_nodes = [
        DeployNode(vm, [pg, g1])
    ]

    return all_nodes
