project(tos)

add_library(lx106_usart usart.hpp src/usart.cpp src/uart_register.h src/uart_priv.h)
target_link_libraries(lx106_usart PRIVATE tos_core)