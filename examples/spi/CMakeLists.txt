project(tos)

add_executable(master master.cpp)
target_link_libraries(master PUBLIC tos_core tos_spi avr_spi)

add_executable(slave slave.cpp)
target_link_libraries(slave PUBLIC tos_core tos_spi avr_spi)
