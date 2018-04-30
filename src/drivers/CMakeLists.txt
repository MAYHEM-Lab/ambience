project(tos)

if (TOS_AVR)
    add_subdirectory(arch/avr)
elseif(TOS_ESP)
    add_subdirectory(arch/lx106)
elseif(TOS_X86)
    add_subdirectory(arch/x86)
endif()

if (NOT TOS_X86)
    add_library(tos_spi_sd src/spi_sd.cpp common/spi_sd.hpp)
    target_link_libraries(tos_spi_sd PUBLIC tos_core tos_spi tos_gpio)
    target_include_directories(tos_spi_sd PRIVATE .)

    add_library(tos_spi common/spi.hpp src/stub.cpp common/timer.hpp src/spi.cpp)
    target_link_libraries(tos_spi PUBLIC tos_gpio tos_core)
    target_include_directories(tos_spi PUBLIC .)

    add_library(tos_usart common/usart.hpp src/stub.cpp)
    target_include_directories(tos_usart PUBLIC .)

    add_library(tos_gpio common/gpio.hpp src/stub.cpp)
    target_include_directories(tos_gpio PUBLIC .)
endif()

add_library(tos_tty common/tty.hpp src/stub.cpp)
target_include_directories(tos_tty PUBLIC .)