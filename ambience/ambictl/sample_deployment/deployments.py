deploy(
    node="hosted",
    groups=["echo"]
)

deploy(
    node="sfo2_vm1",
    groups=["fs", "calc2"]
)

deploy(
    node="sfo2_vm2",
    groups=["", "calc3"]
)

deploy(
    node="mcu1",
    groups=["", "calc"]
)

deploy(
    node="qemu_vm1",
    groups=["fs2", "calc4"]
)