add_library(nrf52_gpio gpio.hpp src/gpio.cpp)
target_link_libraries(nrf52_gpio PUBLIC tos_util tos_gpio tos_core)
target_include_directories(nrf52_gpio PUBLIC .)

add_library(nrf52_usart usart.hpp src/usart.cpp
        ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uarte.c
        ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uart.c)
target_link_libraries(nrf52_usart PUBLIC tos_core)
target_include_directories(nrf52_usart PUBLIC .)
target_compile_definitions(nrf52_usart PRIVATE NRFX_UARTE_ENABLED=1 NRFX_UARTE0_ENABLED=1 UART_LEGACY_SUPPORT=0 NRFX_UART_ENABLED=0)

add_library(nrf52_timer timer.hpp src/timer.cpp
        ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_timer.c)
target_link_libraries(nrf52_timer PUBLIC tos_core)
target_include_directories(nrf52_timer PUBLIC .)
target_compile_definitions(nrf52_timer PRIVATE NRFX_TIMER_ENABLED=1 NRFX_TIMER0_ENABLED=1 TIMER_LEGACY_SUPPORT=0)

add_library(nrf52_radio radio.hpp src/radio.cpp)
target_link_libraries(nrf52_radio PUBLIC tos_core)
target_include_directories(nrf52_radio PUBLIC .)

add_library(nrf52_drivers)
target_link_libraries(nrf52_drivers PUBLIC nrf52_gpio nrf52_usart nrf52_timer nrf52_radio)