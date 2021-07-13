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
)

serv_instance(
    name="calc2",
    serv=basic_calc
)

serv_instance(
    name="echo",
    serv=basic_echo
)
