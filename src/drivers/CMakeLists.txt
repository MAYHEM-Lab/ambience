project(tos)

add_subdirectory(arch/avr)

add_library(tos_spi_sd src/spi_sd.cpp common/spi_sd.hpp)
target_link_libraries(tos_spi_sd PUBLIC tos_core)
target_include_directories(tos_spi_sd PRIVATE .)

add_library(tos_spi common/spi.hpp src/stub.cpp common/timer.hpp)
target_include_directories(tos_spi PUBLIC .)