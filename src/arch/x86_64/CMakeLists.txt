add_library(arch_x86_64 include/tos/x86_64/spmanip.hpp arch.cpp include/tos/x86_64/assembly.hpp include/tos/x86_64/interrupts.hpp include/tos/x86_64/idt.hpp)
target_include_directories(arch_x86_64 PUBLIC include)
target_link_libraries(arch_x86_64 PUBLIC tos_util_core)
target_compile_features(arch_x86_64 PUBLIC cxx_std_20)