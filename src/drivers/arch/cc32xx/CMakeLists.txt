add_library(cc32xx_timer include/arch/timer.hpp src/stub.cpp src/timer.cpp)
target_link_libraries(cc32xx_timer PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_timer PUBLIC cxx_std_17)
target_include_directories(cc32xx_timer PUBLIC include)

add_library(cc32xx_uart include/arch/uart.hpp src/uart.cpp)
target_link_libraries(cc32xx_uart PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_uart PUBLIC cxx_std_17)
target_include_directories(cc32xx_uart PUBLIC include)

add_library(cc32xx_gpio include/arch/gpio.hpp src/stub.cpp)
target_link_libraries(cc32xx_gpio PUBLIC tos_core ti_drivers)
target_compile_features(cc32xx_gpio PUBLIC cxx_std_17)
target_include_directories(cc32xx_gpio PUBLIC include)

add_library(cc32xx_drivers INTERFACE)
target_link_libraries(cc32xx_drivers
    INTERFACE
        cc32xx_timer
        cc32xx_uart
        cc32xx_gpio
)

add_library(arch_drivers ALIAS cc32xx_drivers)