project(tos)

add_executable(temp main.cpp)

target_link_libraries(temp PUBLIC tos_core avr_usart avr_timer tos_print tos_dht22)
target_compile_options(temp PUBLIC "-fwhole-program")