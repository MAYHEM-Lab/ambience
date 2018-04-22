project(tos)

add_subdirectory(arch/avr)

add_library(tos_spi_sd src/spi_sd.cpp common/spi_sd.hpp)
target_link_libraries(tos_spi_sd PUBLIC tos_core tos_spi tos_gpio)
target_include_directories(tos_spi_sd PRIVATE .)

add_library(tos_spi common/spi.hpp src/stub.cpp common/timer.hpp)
target_link_libraries(tos_spi PUBLIC tos_gpio)
target_include_directories(tos_spi PUBLIC .)

add_library(tos_usart common/usart.hpp src/stub.cpp)
target_include_directories(tos_usart PUBLIC .)

add_library(tos_gpio common/gpio.hpp src/stub.cpp)
target_include_directories(tos_gpio PUBLIC .)