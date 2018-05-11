project(tos)

add_executable(exint main.cpp)

target_link_libraries(exint PUBLIC tos_core avr_usart avr_timer tos_print avr_gpio avr_eeprom)
target_compile_options(exint PUBLIC "-fwhole-program")