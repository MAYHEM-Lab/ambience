
add_library(avr_gpio gpio.hpp gpio.cpp)
target_include_directories(avr_gpio PUBLIC .)

add_library(avr_usart usart.cpp usart.hpp)
target_link_libraries(avr_usart PUBLIC tos_core)
target_include_directories(avr_usart PUBLIC .)