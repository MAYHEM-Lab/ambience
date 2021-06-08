add_library(tos_ebpf ebpf.cpp vm.cpp verify.cpp include/tos/ebpf/verify.hpp include/tos/caplets/ebpf_constraint.hpp)
target_include_directories(tos_ebpf PUBLIC include)
target_link_libraries(tos_ebpf PUBLIC tos_util_core)

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(sample)

include(lidlc)
add_lidlc(ebpf_constraint_schema ebpf_constraint.lidl)

add_library(caplets_ebpf_constraint ebpf_constraint.cpp)
target_link_libraries(caplets_ebpf_constraint PUBLIC tos_caplets tos_ebpf ebpf_constraint_schema)