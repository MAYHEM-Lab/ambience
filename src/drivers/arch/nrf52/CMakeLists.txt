add_library(nrf52_gpio include/arch/gpio.hpp src/gpio.cpp)
target_link_libraries(nrf52_gpio PUBLIC tos_util tos_gpio tos_core)
tos_install(nrf52_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_usart include/arch/usart.hpp src/usart.cpp
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uarte.c
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_uart.c)
target_link_libraries(nrf52_usart PUBLIC tos_core)
tos_install(nrf52_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(nrf52_usart PRIVATE NRFX_UARTE_ENABLED=1 NRFX_UARTE0_ENABLED=1 UART_LEGACY_SUPPORT=0 NRFX_UART_ENABLED=0)

add_library(nrf52_timer include/arch/timer.hpp src/timer.cpp
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_timer.c)
target_link_libraries(nrf52_timer PUBLIC tos_core)
tos_install(nrf52_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(nrf52_timer PRIVATE NRFX_TIMER_ENABLED=1 NRFX_TIMER0_ENABLED=1 TIMER_LEGACY_SUPPORT=0)

add_library(nrf52_twim
    ${SDK_ROOT}/modules/nrfx/drivers/src/nrfx_twim.c include/arch/twim.hpp src/twim.cpp)
target_link_libraries(nrf52_twim PUBLIC tos_core)
tos_install(nrf52_twim ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_definitions(nrf52_twim PRIVATE
    NRFX_TWIM_ENABLED=1
    NRFX_TWIM0_ENABLED=1
    TWIM_LEGACY_SUPPORT=0)

add_library(nrf52_radio include/arch/radio.hpp src/radio.cpp)
target_link_libraries(nrf52_radio PUBLIC tos_core)
tos_install(nrf52_radio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(nrf52_drivers src/stub.cpp include/arch/drivers.hpp)
target_link_libraries(nrf52_drivers PUBLIC
    nrf52_gpio
    nrf52_usart
    nrf52_timer
    nrf52_radio
    nrf52_twim
)
tos_install(nrf52_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(nrf52_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS nrf52_drivers)