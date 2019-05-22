add_library(stm32_rcc include/arch/rcc.hpp src/stub.cpp)
target_link_libraries(stm32_rcc PUBLIC tos::core)
tos_install(stm32_rcc ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_gpio include/arch/gpio.hpp src/stub.cpp)
target_link_libraries(stm32_gpio PUBLIC tos_gpio tos::core)
tos_install(stm32_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_usart include/arch/usart.hpp src/stub.cpp src/usart.cpp)
target_link_libraries(stm32_usart PUBLIC tos_usart tos::core)
tos_install(stm32_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_timer include/arch/timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(stm32_timer PUBLIC tos::core)
tos_install(stm32_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_spi include/arch/spi.hpp src/stub.cpp)
target_link_libraries(stm32_spi PUBLIC tos::core)
tos_install(stm32_spi ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_i2c src/stub.cpp)
target_link_libraries(stm32_i2c PUBLIC tos::core)
tos_install(stm32_i2c ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_adc include/arch/adc.hpp src/stub.cpp src/adc.cpp)
target_link_libraries(stm32_adc PUBLIC tos::core)
tos_install(stm32_adc ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_drivers include/arch/drivers.hpp src/stub.cpp)
target_link_libraries(stm32_drivers PUBLIC
        stm32_gpio
        stm32_usart
        stm32_rcc
        stm32_timer
        stm32_i2c
        stm32_adc
        stm32_spi
        )
target_link_libraries(stm32_drivers PUBLIC "-Wl,--whole-archive $<TARGET_FILE:stm32_timer> $<TARGET_FILE:stm32_usart> -Wl,--no-whole-archive")
tos_install(stm32_drivers)

set_target_properties(stm32_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS stm32_drivers)