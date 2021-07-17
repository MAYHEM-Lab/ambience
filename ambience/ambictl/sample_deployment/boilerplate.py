import ambictl

calculator_mod = ambictl.LidlModule("//ambience/services/interfaces/calc.lidl", "calc_schema")
alarm_mod = ambictl.LidlModule("//ambience/services/interfaces/alarm.lidl", "alarm_schema")
fs_mod = ambictl.LidlModule("//ambience/services/interfaces/file_system.lidl", "filesystem_schema")
block_mem_mod = ambictl.LidlModule("//ambience/services/interfaces/block_memory.lidl", "block_memory_schema")
machine_mod = ambictl.LidlModule("//ambience/services/interfaces/machine.lidl", "machine_schema")
echo_mod = ambictl.LidlModule("//ambience/services/interfaces/echo.lidl", "echo_schema")
db_mod = ambictl.LidlModule("//ambience/services/interfaces/database.lidl", "database_schema")
logger_mod = ambictl.LidlModule("//src/core/log.yaml", "log_schema")
agent_mod = ambictl.LidlModule("//ambience/services/interfaces/agent.lidl", "agent_schema")
sm_common_mod = ambictl.LidlModule("//ambience/services/impls/social_media/sm_common.lidl", "social_media_schema")
accounts_mod = ambictl.LidlModule("//ambience/services/impls/social_media/accounts.lidl", "social_media_schema")
posts_mod = ambictl.LidlModule("//ambience/services/impls/social_media/posts.lidl", "social_media_schema")
analysis_mod = ambictl.LidlModule("//ambience/services/impls/social_media/analysis.lidl", "social_media_schema")

calc_if = calculator_mod.get_service("tos::ae::services::calculator")
logger_if = logger_mod.get_service("tos::services::logger")
alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
fs_if = fs_mod.get_service("tos::ae::services::filesystem")
machine_if = machine_mod.get_service("tos::ae::services::machine")
block_mem_if = block_mem_mod.get_service("tos::ae::services::block_memory")
echo_if = echo_mod.get_service("tos::ae::services::echo")
db_if = fs_mod.get_service("tos::ae::services::sql_database")
agent_if = agent_mod.get_service("tos::ae::agent")
post_analysis_if = analysis_mod.get_service("social_media::post_analysis")
posts_if = posts_mod.get_service("social_media::posts")

basic_echo = echo_if.implement(
    name="basic_echo",
    cmake_target="basic_echo",
    sync=True,
    deps={
        "logger": logger_if
    }
)

littlefs = fs_if.implement(
    name="littlefs_server",
    cmake_target="littlefs_server",
    sync=True,
    deps={
        "block": block_mem_if
    }
)

basic_calc = calc_if.implement(
    name="basic_calc",
    cmake_target="basic_calc",
    sync=False,
    deps={
        "logger": logger_if,
        "alarm": alarm_if,
        "fs": fs_if
    }
)

calc_bench_agent = agent_if.implement(
    name="calc_bench_agent",
    cmake_target="calc_bench_agent",
    sync=True,
    deps={
        "calc": calc_if
    }
)

async_calc_bench_agent = agent_if.implement(
    name="async_calc_bench_agent",
    cmake_target="calc_bench_agent",
    sync=False,
    deps={
        "calc": calc_if
    }
)

basic_analyzer = post_analysis_if.implement(
    name="basic_analyzer",
    cmake_target="basic_analyzer",
    sync=False,
    deps={}
)

posts_manager = posts_if.implement(
    name="posts_manager",
    cmake_target="posts_manager",
    sync=False,
    deps={
        "analysis": post_analysis_if
    }
)

posts_bench_agent = agent_if.implement(
    name="posts_bench_agent",
    cmake_target="posts_bench_agent",
    sync=False,
    deps={
        "posts": posts_if
    }
)
