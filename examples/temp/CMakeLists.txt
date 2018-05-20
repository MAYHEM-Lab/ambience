project(tos)

add_executable(temp main.cpp)

add_executable(temp_master master.cpp)

target_link_libraries(temp PUBLIC tos_core avr_usart avr_timer tos_print tos_dht22 avr_gpio avr_spi)
target_compile_options(temp PUBLIC "-fwhole-program")

target_link_libraries(temp_master PUBLIC tos_core avr_usart avr_timer tos_alarm tos_print avr_gpio)
target_compile_options(temp_master PUBLIC "-fwhole-program")