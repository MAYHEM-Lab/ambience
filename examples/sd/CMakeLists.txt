project(tos)

add_executable(sdcard main.cpp)

target_link_libraries(sdcard PUBLIC tos_core tos_spi_sd avr_usart avr_spi avr_timer tos_print)
target_compile_options(sdcard PUBLIC "-fwhole-program")