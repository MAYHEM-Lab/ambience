add_service(dns_servers dns dns.cpp)

if (TARGET ae_user_space)
    add_service(dns_bench_agent agent agent.cpp)
    target_link_libraries(dns_bench_agent PUBLIC dns_schema ae_user_space)
endif()