project(tos)

add_executable(alarm main.cpp)

target_link_libraries(alarm PUBLIC tos_core tos_alarm avr_usart avr_timer tos_print avr_gpio avr_eeprom)
target_compile_options(alarm PUBLIC "-fwhole-program")