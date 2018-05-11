add_library(avr_gpio gpio.hpp src/gpio.cpp)
target_link_libraries(avr_gpio PUBLIC tos_util tos_gpio tos_core)
target_include_directories(avr_gpio PUBLIC .)

add_library(avr_usart src/usart.cpp usart.hpp)
target_link_libraries(avr_usart PUBLIC tos_core)
target_include_directories(avr_usart PUBLIC .)

add_library(avr_spi spi.hpp src/spi.cpp)
target_link_libraries(avr_spi PUBLIC tos_core avr_gpio tos_spi)
target_include_directories(avr_spi PUBLIC .)

add_library(avr_timer timer.hpp src/timer.cpp)
target_link_libraries(avr_timer PUBLIC tos_core)
target_include_directories(avr_timer PUBLIC .)

add_library(avr_eeprom eeprom.hpp src/eeprom.cpp)
target_link_libraries(avr_eeprom PUBLIC tos_core tos_eeprom)
target_include_directories(avr_eeprom PUBLIC .)