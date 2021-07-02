add_executable(stm32_spi_slave slave.cpp)
target_link_libraries(stm32_spi_slave PUBLIC tos::core tos::print tos_board)

add_executable(stm32_spi_master master.cpp)
target_link_libraries(stm32_spi_master PUBLIC tos::core tos::print tos_board)