project(tos CXX ASM)

add_library(avr_impl avr_impl.cpp tos_arch.hpp tos_platform.hpp)
target_include_directories(avr_impl PRIVATE "../../")
target_include_directories(avr_impl PUBLIC ".")
target_link_libraries(avr_impl PUBLIC tos_core)
