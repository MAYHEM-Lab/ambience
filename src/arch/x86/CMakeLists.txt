add_library(arch_x86 include/tos/x86/spmanip.hpp arch.cpp include/tos/x86/assembly.hpp)
target_include_directories(arch_x86 PUBLIC include)
target_link_libraries(arch_x86 PUBLIC tos_util_core)