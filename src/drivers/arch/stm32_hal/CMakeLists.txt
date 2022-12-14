add_library(stm32_gpio include/arch/gpio.hpp src/stub.cpp)
target_link_libraries(stm32_gpio PUBLIC tos_gpio tos::drivers)
tos_install(stm32_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_timer include/arch/timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(stm32_timer PUBLIC tos::drivers)
tos_install(stm32_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_options(stm32_timer PUBLIC "-Wl,-uHAL_TIM_IRQHandler")

add_library(stm32_usart include/arch/usart.hpp src/usart.cpp)
target_link_libraries(stm32_usart PUBLIC tos_usart tos::drivers)
tos_install(stm32_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_options(stm32_timer PUBLIC "-Wl,-uHAL_UART_IRQHandler")

add_library(stm32_spi include/arch/spi.hpp src/spi.cpp)
target_link_libraries(stm32_spi PUBLIC tos::drivers)
target_link_options(stm32_spi PUBLIC "-Wl,-uHAL_SPI_IRQHandler")
tos_install(stm32_spi ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_i2c include/arch/i2c.hpp src/i2c.cpp)
target_link_libraries(stm32_i2c PUBLIC tos::drivers tos_i2c)
tos_install(stm32_i2c ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_exti include/arch/exti.hpp src/exti.cpp)
target_link_libraries(stm32_exti PUBLIC tos::drivers etl tos_debug)
tos_install(stm32_exti ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_flash include/arch/flash.hpp src/flash.cpp)
target_link_libraries(stm32_flash PUBLIC tos::drivers)
tos_install(stm32_flash ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(stm32_drivers PUBLIC
        stm32_gpio
        stm32_timer
        stm32_usart
        stm32_spi
        stm32_i2c
        stm32_exti
        stm32_flash
        )
target_link_libraries(stm32_drivers PUBLIC "--whole-archive $<TARGET_FILE:stm32_timer> $<TARGET_FILE:stm32_usart> $<TARGET_FILE:stm32_spi> $<TARGET_FILE:stm32_exti> --no-whole-archive")
tos_install(stm32_drivers)

set_target_properties(stm32_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS stm32_drivers)