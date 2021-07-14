deploy(
    node="hosted",
    groups=["echo"]
)

deploy(
    node="sfo2_vm1",
    groups=["fs", "calc2"]
)

group(
    name="sfo_vm2_priv",
    services=["bench_agent"]
)

group(
    name="sfo_vm2_agent2",
    services=["bench_agent2"]
)

group(
    name="sfo2_vm_calc_user",
    services=["calc3"]
)

deploy(
    node="sfo2_vm2",
    groups=["sfo_vm2_priv", "sfo2_vm_calc_user", "sfo_vm2_agent2"]
)

deploy(
    node="mcu1",
    groups=["", "calc"]
)

# deploy(
#     node="qemu_vm1",
#     groups=["fs2", "calc4"]
# )