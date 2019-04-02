project(tos CXX ASM)

add_library(avr_core avr_core.cpp include/tos_arch.hpp include/tos_platform.hpp include/sfr.hpp)
add_library(avr_impl avr_impl.cpp)

target_link_libraries(avr_core PUBLIC tos_util_core)
target_link_libraries(avr_impl PUBLIC avr_core tos_ft)

tos_install(avr_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(avr_core ${CMAKE_CURRENT_SOURCE_DIR}/include)