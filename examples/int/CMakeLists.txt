project(tos)

add_executable(exint main.cpp)

target_link_libraries(exint PUBLIC tos_core tos_print avr_drivers)