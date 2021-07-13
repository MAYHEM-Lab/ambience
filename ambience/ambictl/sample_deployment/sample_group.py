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
    name="basic_echo",
    serv=basic_echo
)

deploy(
    node="hosted",
    groups=["", "basic_echo"]
)

deploy(
    node="sfo2-vm1",
    groups=["fs", "calc", "calc2"]
)

deploy(
    node="mcu1",
    groups=[]
)

