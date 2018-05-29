project(tos)

add_executable(nrf24_demo main.cpp)
target_link_libraries(nrf24_demo PUBLIC tos_core tos_print avr_usart tos_tty tos_nrf24 avr_spi avr_timer)