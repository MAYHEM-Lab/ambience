add_library(nrf52_gpio gpio.hpp src/gpio.cpp)
target_link_libraries(nrf52_gpio PUBLIC tos_util tos_gpio tos_core)
target_include_directories(nrf52_gpio PUBLIC .)

add_library(nrf52_usart usart.hpp src/usart.cpp)
target_link_libraries(nrf52_usart PUBLIC tos_core)
target_include_directories(nrf52_usart PUBLIC .)

add_library(nrf52_drivers)
target_link_libraries(nrf52_drivers PUBLIC nrf52_gpio)