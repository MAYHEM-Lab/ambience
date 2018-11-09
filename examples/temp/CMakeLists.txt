project(tos)

add_executable(temp slave.cpp app.hpp)

add_executable(temp_master master.cpp app.hpp)

target_link_libraries(temp PUBLIC tos_core tos_print tos_dht22 avr_drivers)
target_link_libraries(temp_master PUBLIC tos_core tos_alarm tos_print avr_drivers tos_xbee tos::ina219)
