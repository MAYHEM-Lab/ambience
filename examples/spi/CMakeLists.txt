project(tos)

add_executable(spi_master master.cpp)
target_link_libraries(spi_master PUBLIC tos::core tos_spi avr_drivers)

add_executable(spi_slave slave.cpp)
target_link_libraries(spi_slave PUBLIC tos::core tos_spi avr_drivers)
