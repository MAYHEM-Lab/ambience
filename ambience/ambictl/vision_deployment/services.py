serv_instance(
    name="calc",
    serv=basic_calc,
    deps={
    }
)

export(
    service="calc",
    networks={
        "udp-internet": 9993
    }
)