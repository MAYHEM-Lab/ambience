include(lidlc)
add_lidlc(ebpf_test_schema ebpf_test.lidl)

add_library(ebpf_test OBJECT test.cpp)
target_link_libraries(ebpf_test PRIVATE ebpf_test_schema)
