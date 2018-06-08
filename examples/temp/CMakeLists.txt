project(tos)

add_executable(temp main.cpp)

add_executable(temp_master master.cpp)

target_link_libraries(temp PUBLIC tos_core tos_print tos_dht22 avr_drivers)
target_link_libraries(temp_master PUBLIC tos_core tos_alarm tos_print avr_drivers)
