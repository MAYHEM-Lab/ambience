add_executable(nrf52_blink main.cpp ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uarte.c ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uart.c)
target_link_libraries(nrf52_blink PUBLIC tos_core)
target_compile_definitions(nrf52_blink PUBLIC NRFX_UARTE_ENABLED=1 NRFX_UARTE0_ENABLED=1 UART_LEGACY_SUPPORT=0 NRFX_UART_ENABLED=0)