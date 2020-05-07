project(tos CXX ASM)

add_library(atmega_core avr_core.cpp include/tos_arch.hpp include/tos_platform.hpp include/sfr.hpp)
add_library(atmega_impl avr_impl.cpp)

target_link_libraries(atmega_core PUBLIC tos_util_core)
target_link_libraries(atmega_impl PUBLIC atmega_core tos_ft)

tos_install(atmega_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(atmega_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)