serv_instance(
    name="calc",
    serv=basic_calc,
    deps={
    }
)

serv_instance(
    name="bench_agent",
    serv=async_calc_bench_agent,
    deps={
        "calc": "calc"
    }
)

export(
    service="bench_agent",
    networks={
        "xbee-home": 0
    }
)