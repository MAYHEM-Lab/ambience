add_library(arch_xtensa
        arch.cpp include/tos/xtensa/assembly.hpp include/tos/xtensa/processor_state.hpp)
target_include_directories(arch_xtensa PUBLIC include)
target_link_libraries(arch_xtensa PUBLIC tos_util_core tos::libcxx)