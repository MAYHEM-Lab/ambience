project(tos)

add_subdirectory(arch/avr)

add_library(tos_spi_sd src/spi_sd.cpp spi_sd.hpp)
target_link_libraries(tos_spi_sd PUBLIC tos_core)
target_include_directories(tos_spi_sd PRIVATE .)