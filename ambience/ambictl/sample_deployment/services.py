serv_instance(
    name="fs",
    serv=littlefs,
    deps={
        "block": "fs_block.local"
    }
)

serv_instance(
    name="fs2",
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
    name="calc4",
    serv=basic_calc,
    deps={
        "logger": node_local,
        "alarm": node_local,
        "fs": "fs2"
    }
)

export(
    service="calc4",
    networks={
        "udp-internet": 1893
    }
)