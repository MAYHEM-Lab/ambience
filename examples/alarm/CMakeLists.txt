project(tos)

add_executable(alarm main.cpp)

target_link_libraries(alarm PUBLIC tos_core tos_alarm tos_print avr_drivers)