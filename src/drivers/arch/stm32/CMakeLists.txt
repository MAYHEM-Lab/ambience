add_library(stm32_rcc rcc.hpp src/stub.cpp)
target_link_libraries(stm32_rcc PUBLIC tos::core)
target_include_directories(stm32_rcc PUBLIC ".")

add_library(stm32_gpio gpio.hpp src/stub.cpp)
target_link_libraries(stm32_gpio PUBLIC tos_gpio tos::core)
target_include_directories(stm32_gpio PUBLIC ".")

add_library(stm32_usart usart.hpp src/stub.cpp)
target_link_libraries(stm32_usart PUBLIC tos_usart tos::core)
target_include_directories(stm32_usart PUBLIC ".")

add_library(stm32_timer timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(stm32_timer PUBLIC tos::core)
target_include_directories(stm32_timer PUBLIC ".")

add_library(stm32_drivers drivers.hpp src/stub.cpp)
target_link_libraries(stm32_drivers PUBLIC
        stm32_gpio
        stm32_usart
        stm32_rcc
        stm32_timer
        )
target_link_libraries(stm32_drivers PUBLIC "-Wl,--whole-archive $<TARGET_FILE:stm32_timer> -Wl,--no-whole-archive")