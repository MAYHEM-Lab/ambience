
add_library(stm32_gpio gpio.hpp src/stub.cpp)
target_link_libraries(stm32_gpio PUBLIC tos_gpio tos::core)
target_include_directories(stm32_gpio PUBLIC ".")

add_library(stm32_timer timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(stm32_timer PUBLIC tos::core)
target_include_directories(stm32_timer PUBLIC ".")

add_library(stm32_drivers drivers.hpp src/stub.cpp)
target_link_libraries(stm32_drivers PUBLIC
        stm32_gpio
        stm32_timer
        )