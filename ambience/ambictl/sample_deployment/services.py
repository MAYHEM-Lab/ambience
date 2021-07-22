serv_instance(
    name="fs",
    serv=littlefs,
    deps={
        "block": "fs_block.localnode"
    }
)

serv_instance(
    name="fs_blok",
    serv=ephemeral_block,
    deps={}
)

serv_instance(
    name="fs2",
    serv=littlefs,
    deps={
        "block": "fs_blok"
    }
)

serv_instance(
    name="calc",
    serv=basic_calc,
    deps={
        "logger": node_local,
        "alarm": node_local,
        "fs": "fs",
    }
)

serv_instance(
    name="calc3",
    serv=basic_calc,
    deps={
        "logger": node_local,
        "alarm": node_local,
        "fs": "fs"
    }
)

serv_instance(
    name="calc2",
    serv=basic_calc,
)

serv_instance(
    name="echo",
    serv=basic_echo
)

serv_instance(
    name="bench_agent",
    serv=calc_bench_agent,
    deps={
        "calc": "calc2"
    }
)

serv_instance(
    name="bench_agent2",
    serv=async_calc_bench_agent,
    deps={
        "calc": "calc3"
    }
)

serv_instance(
    name="analysis",
    serv=basic_analyzer
)

serv_instance(
    name="posts",
    serv=posts_manager,
    deps={
        "analysis": "analysis"
    }
)

serv_instance(
    name="posts_agent",
    serv=posts_bench_agent,
    deps={
        "posts": "posts",
        "fs": "fs2"
    }
)

# export(
#     service="bench_agent",
#     networks={
#         "udp-internet": 1894
#     }
# )
#
# export(
#     service="bench_agent2",
#     networks={
#         "udp-internet": 1895
#     }
# )

export(
    service="posts_agent",
    networks={
        "udp-internet": 1896
    }
)