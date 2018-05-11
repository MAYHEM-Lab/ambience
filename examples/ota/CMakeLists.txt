project(tos)

add_executable(ota main.cpp)

target_link_libraries(ota PUBLIC tos_core avr_usart avr_timer tos_print)
target_compile_options(ota PUBLIC "-fwhole-program")