add_executable(ebpf_demo main.cpp)
target_link_libraries(ebpf_demo PRIVATE tos_core tos_ebpf tos_board policy_schema)