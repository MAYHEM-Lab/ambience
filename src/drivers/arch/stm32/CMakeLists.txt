add_library(stm32_rcc include/arch/stm32/rcc.hpp src/stub.cpp)
target_link_libraries(stm32_rcc PUBLIC tos::core)
tos_install(stm32_rcc ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_gpio include/arch/stm32/gpio.hpp src/stub.cpp)
target_link_libraries(stm32_gpio PUBLIC tos_gpio tos::core)
tos_install(stm32_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_usart include/arch/stm32/usart.hpp src/stub.cpp src/usart.cpp)
target_link_libraries(stm32_usart PUBLIC tos_usart tos::core)
tos_install(stm32_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_timer include/arch/stm32/timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(stm32_timer PUBLIC tos::core)
tos_install(stm32_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_i2c include/arch/stm32/i2c.hpp src/stub.cpp)
target_link_libraries(stm32_i2c PUBLIC tos::core)
tos_install(stm32_i2c ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_drivers include/arch/stm32/drivers.hpp src/stub.cpp)
target_link_libraries(stm32_drivers PUBLIC
        stm32_gpio
        stm32_usart
        stm32_rcc
        stm32_timer
        stm32_i2c
        )
target_link_libraries(stm32_drivers PUBLIC "-Wl,--whole-archive $<TARGET_FILE:stm32_timer> $<TARGET_FILE:stm32_usart> -Wl,--no-whole-archive")
tos_install(stm32_drivers)
