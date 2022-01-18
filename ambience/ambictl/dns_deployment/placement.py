group(
    name="sync_recursive_group",
    services=["sync_user_group", "sync_recursive"]
)

group(
    name="async_recursive_group",
    services=["async_user_group", "async_recursive", "async_bench_agent"]
)

deploy(
    node="vm",
    groups=["", "final_dns1", "final_dns2", "sync_recursive_group",
            "bench_agent", "async_recursive_group"]
)
