add_library(arch_riscv
        include/tos/riscv/spmanip.hpp include/tos/riscv/assembly.hpp riscv.cpp include/tos/riscv/interrupts.hpp)
target_include_directories(arch_riscv PUBLIC include)
target_link_libraries(arch_riscv PUBLIC tos_util_core tos::libcxx)
target_compile_features(arch_riscv PUBLIC cxx_std_20)