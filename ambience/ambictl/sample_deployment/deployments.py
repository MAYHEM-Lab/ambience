deploy(
    node="hosted",
    groups=["echo"]
)

deploy(
    node="sfo2-vm1",
    groups=["fs", "calc", "calc2"]
)

deploy(
    node="mcu1",
    groups=[]
)

