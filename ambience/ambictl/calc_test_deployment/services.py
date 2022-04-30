serv_instance(
    name="calc",
    serv=basic_calc,
    deps={
    }
)

serv_instance(
    name="echo",
    serv=basic_echo,
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
    service="echo",
    networks={
        "rest-http": "/echo"
    }
)

export(
    service="calc",
    networks={
        "rest-http": "/calc"
    }
)

export(
    service="bench_agent",
    networks={
        "udp-internet": 1234,
        "rest-http": "/bench_agent",
    }
)