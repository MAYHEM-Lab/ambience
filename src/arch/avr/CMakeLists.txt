add_library(arch_avr include/tos/avr/spmanip.hpp include/tos/avr/interrupts.hpp arch.cpp include/tos/avr/assembly.hpp)
target_include_directories(arch_avr PUBLIC include)
target_link_libraries(arch_avr PUBLIC tos_util_core)