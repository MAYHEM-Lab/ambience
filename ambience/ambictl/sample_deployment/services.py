serv_instance(
    name="fs",
    serv=littlefs,
    deps={
        "block": "fs_block.local"
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

export(
    service="bench_agent",
    networks={
        "udp-internet": 1894
    }
)

export(
    service="bench_agent2",
    networks={
        "udp-internet": 1895
    }
)