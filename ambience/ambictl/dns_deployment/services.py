serv_instance(
    name="final_dns1",
    serv=final_dns_resolver,
    deps={
    }
)

serv_instance(
    name="final_dns2",
    serv=final_dns_resolver,
    deps={
    }
)

serv_instance(
    name="sync_recursive",
    serv=sync_recursive_dns_resolver,
    deps={
        "b1": "final_dns1",
        "b2": "final_dns2",
    }
)

serv_instance(
    name="async_recursive",
    serv=async_recursive_dns_resolver,
    deps={
        "b1": "final_dns1",
        "b2": "final_dns2",
    }
)

serv_instance(
    name="bench_agent",
    serv=dns_bench_agent,
    deps={
        "dns": "sync_recursive"
    }
)

export(
    service="bench_agent",
    networks={
        "udp-internet": 1234,
    }
)

serv_instance(
    name="async_bench_agent",
    serv=dns_bench_agent,
    deps={
        "dns": "async_recursive"
    }
)

export(
    service="async_bench_agent",
    networks={
        "udp-internet": 1235,
    }
)

# export(
#     service="async_recursive",
#     networks={
#         "udp-internet": 1235,
#     }
# )