add_library(arch_i386 include/tos/i386/spmanip.hpp arch.cpp include/tos/i386/assembly.hpp include/tos/i386/mmu.hpp)
target_include_directories(arch_i386 PUBLIC include)
target_link_libraries(arch_i386 PUBLIC tos_util_core clang_rt.builtins-i386)