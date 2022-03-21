serv_instance(
    name="poll",
    serv=basic_poll,
    deps={}
)

serv_instance(
    name="poll_bench_agent",
    serv=async_poll_bench_agent,
    deps={
        "poll": "poll"
    }
)

export(
    service="poll_bench_agent",
    networks={
        "udp-internet": 1234,
    }
)