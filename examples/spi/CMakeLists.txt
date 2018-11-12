project(tos)

add_executable(master master.cpp)
target_link_libraries(master PUBLIC tos::core tos_spi avr_drivers)

add_executable(slave slave.cpp)
target_link_libraries(slave PUBLIC tos::core tos_spi avr_drivers)
