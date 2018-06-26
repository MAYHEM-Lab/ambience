project(tos)

add_library(lx106_usart usart.hpp src/usart.cpp src/uart_register.h src/uart_priv.h)
target_link_libraries(lx106_usart PRIVATE tos_core lx106_impl)

add_library(esp82_timer timer.hpp src/timer.cpp)
target_include_directories(esp82_timer PUBLIC .)
target_compile_definitions(esp82_timer PUBLIC USE_US_TIMER)
target_link_libraries(esp82_timer PRIVATE tos_core lx106_impl)

add_library(esp82_drivers src/stub.cpp)
target_link_libraries(esp82_drivers PUBLIC lx106_usart esp82_timer)