project(tos)

add_library(lx106_usart usart.hpp src/uart_register.h src/new_uart_priv.h src/uart_driver.cpp)
target_link_libraries(lx106_usart PRIVATE tos_core lx106_impl)

add_library(esp82_timer timer.hpp src/timer.cpp)
target_include_directories(esp82_timer PUBLIC .)
target_compile_definitions(esp82_timer PUBLIC USE_US_TIMER)
target_link_libraries(esp82_timer PRIVATE tos_core lx106_impl)

add_library(esp82_wifi wifi.hpp src/wifi.cpp)
target_include_directories(esp82_wifi PUBLIC .)
target_link_libraries(esp82_wifi PUBLIC tos_core lx106_impl)

add_library(esp82_tcp tcp.hpp src/tcp.cpp)
target_include_directories(esp82_tcp PUBLIC .)
target_link_libraries(esp82_tcp PUBLIC esp82_wifi tos_inet)

add_library(esp82_gpio gpio.hpp src/gpio.cpp)
target_include_directories(esp82_gpio PUBLIC .)
target_link_libraries(esp82_gpio PUBLIC tos_core tos_gpio)

add_library(esp82_drivers src/stub.cpp drivers.hpp)
target_link_libraries(esp82_drivers PUBLIC lx106_usart esp82_timer esp82_wifi esp82_tcp)