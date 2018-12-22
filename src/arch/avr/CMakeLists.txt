project(tos CXX ASM)

add_library(avr_impl avr_impl.cpp include/tos_arch.hpp include/tos_platform.hpp)
target_include_directories(avr_impl PRIVATE "../../")
target_link_libraries(avr_impl PUBLIC tos_core)

tos_install(avr_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)