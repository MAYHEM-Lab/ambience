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
kvstore_mod = ambictl.LidlModule("//ambience/services/impls/social_media/kvstore.lidl", "kvstore_schema")
networking_mod = ambictl.LidlModule("//ambience/services/impls/social_media/networking.lidl", "networking_schema")
weather_sensor_mod = ambictl.LidlModule("//ambience/services/interfaces/weather_sensor.lidl", "weather_sensor_schema")
dns_mod = ambictl.LidlModule("//ambience/services/interfaces/dns.lidl", "dns_schema")

calc_if = calculator_mod.get_service("tos::ae::services::calculator")
logger_if = logger_mod.get_service("tos::services::logger")
alarm_if = alarm_mod.get_service("tos::ae::services::alarm")
fs_if = fs_mod.get_service("tos::ae::services::filesystem")
machine_if = machine_mod.get_service("tos::ae::services::machine")
block_mem_if = block_mem_mod.get_service("tos::ae::services::block_memory")
echo_if = echo_mod.get_service("tos::ae::services::echo")
db_if = fs_mod.get_service("tos::ae::services::sql_database")
agent_if = agent_mod.get_service("tos::ae::agent")
nullaryfn_if = agent_mod.get_service("tos::ae::nullaryfn")
post_analysis_if = analysis_mod.get_service("social_media::post_analysis")
posts_if = posts_mod.get_service("social_media::posts")
kvstore_if = kvstore_mod.get_service("tos::ae::services::KVStore")
bytestore_if = kvstore_mod.get_service("tos::ae::services::ByteStore")
udp_socket_if = networking_mod.get_service("tos::services::udp_socket")
weather_sensor_if = weather_sensor_mod.get_service("tos::ae::services::weather_sensor")
dns_if = dns_mod.get_service("tos::ae::services::dns")

timestamp = nullaryfn_if.implement(
    name="timestamp",
    cmake_target="timestamp",
    sync=False,
    deps={}
)

dns_bench_agent = agent_if.implement(
    name="dns_bench_agent",
    cmake_target="dns_bench_agent",
    sync=False,
    deps={
        "dns": dns_if
    }
)

final_dns_resolver2 = dns_if.implement(
    name="final_dns_resolver2",
    cmake_target="dns_servers",
    sync=False,
    deps={}
)

final_dns_resolver = dns_if.implement(
    name="final_dns_resolver",
    cmake_target="dns_servers",
    sync=False,
    deps={}
)

async_recursive_dns_resolver = dns_if.implement(
    name="async_recursive_dns_resolver",
    cmake_target="dns_servers",
    sync=False,
    deps={
        "b1": dns_if,
        "b2": dns_if,
    }
)

sync_recursive_dns_resolver = dns_if.implement(
    name="sync_recursive_dns_resolver",
    cmake_target="dns_servers",
    sync=True,
    deps={
        "b1": dns_if,
        "b2": dns_if,
    }
)

null_weather_sensor = weather_sensor_if.implement(
    name="null_weather_sensor",
    cmake_target="mock_weather_sensor",
    sync=False,
    deps={}
)

weather_sampler = agent_if.implement(
    name="weather_sampler",
    cmake_target="weather_sampler",
    sync=False,
    deps={
        "alarm": alarm_if,
        "sensor": weather_sensor_if
    }
)

ephemeral_block = block_mem_if.implement(
    name="ephemeral_block",
    cmake_target="ephemeral_block",
    sync=True,
    deps={

    }
)

basic_echo = echo_if.implement(
    name="basic_echo",
    cmake_target="basic_echo",
    sync=True,
    deps={
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

fatih_kvstore = bytestore_if.implement(
    name="fatih_kvstore",
    cmake_target="fatih_kvstore",
    sync=False,
    deps={}
)

inmem_kv = kvstore_if.implement(
    name="inmem_kv",
    cmake_target="inmem_kv",
    sync=False,
    deps={}
)

inmem_bytestore = bytestore_if.implement(
    name="inmem_bytestore",
    cmake_target="inmem_kv",
    sync=False,
    deps={}
)

lwip_udp_socket = udp_socket_if.implement(
    name="lwip_udp_socket",
    cmake_target="lwip_udp_socket",
    sync=False,
    deps={}
)

