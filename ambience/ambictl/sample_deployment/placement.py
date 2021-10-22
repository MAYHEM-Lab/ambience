deploy_to = "end_to_end"

if deploy_to == "linux":
    group(
        name="posts_group",
        services=[
            "analysis",
            "posts",
        ]
    )

    deploy(
        node="hosted",
        groups=["posts_group"]
    )

    # deploy(
    #     node="hosted3",
    #     groups=["analysis"]
    # )

    group(
        name="hosted2_priv",
        services=["fs_blok", "fs2", "posts_agent"]
    )

    deploy(
        node="hosted2",
        groups=["hosted2_priv"]
    )
elif deploy_to == "nrf":
    deploy(
        node="nrf",
        groups=[
            "",
            "posts",
            "analysis",
            "posts_agent"
        ]
    )
elif deploy_to == "end_to_end":
    deploy(
        node="sfo2_vm1",
        groups=[
        ]
    )

    deploy(
        node="sfo2_vm2",
        groups=[
            "",
            "analysis",
        ]
    )

    deploy(
        node="hosted",
        groups=[
            "posts",
        ]
    )

    deploy(
        node="mcu1",
        groups=[
            "",
            "posts_agent"
        ]
    )
else:
    group(
        name="posts_group",
        services=[
            "analysis",
            "posts",
        ]
    )

    deploy(
        node="sfo2_vm1",
        groups=[
            "fs",
            "posts_group"
        ]
    )

    deploy(
        node="sfo2_vm2",
        groups=[
            "fs2",
            # "posts",
            # "analysis",
            "posts_agent"
        ]
    )


# group(
#     name="sfo_vm2_priv",
#     services=["bench_agent"]
# )
#
# group(
#     name="sfo_vm2_agent2",
#     services=["bench_agent2"]
# )
#
# group(
#     name="sfo2_vm_calc_user",
#     services=["calc3"]
# )

# deploy(
#     node="mcu1",
#     groups=["", "calc"]
# )

# deploy(
#     node="qemu_vm1",
#     groups=["fs2", "calc4"]
# )
