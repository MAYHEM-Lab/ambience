add_library(avr_gpio include/arch/gpio.hpp src/gpio.cpp)
target_link_libraries(avr_gpio PUBLIC tos_util tos_gpio tos_core)
tos_install(avr_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_usart src/usart.cpp include/arch/usart.hpp)
target_link_libraries(avr_usart PUBLIC tos_core)
tos_install(avr_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_spi include/arch/spi.hpp src/spi.cpp)
target_link_libraries(avr_spi PUBLIC tos_core avr_gpio tos_spi)
tos_install(avr_spi ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_timer include/arch/timer.hpp src/timer.cpp)
target_link_libraries(avr_timer PUBLIC tos_core)
tos_install(avr_timer ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_eeprom include/arch/eeprom.hpp src/eeprom.cpp)
target_link_libraries(avr_eeprom PUBLIC tos_core tos_eeprom)
tos_install(avr_eeprom ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_twim include/arch/twim.hpp src/twim.cpp)
target_link_libraries(avr_twim PUBLIC tos::core)
tos_install(avr_twim ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_adc include/arch/adc.hpp src/adc.cpp)
target_link_libraries(avr_adc PUBLIC tos::core)
tos_install(avr_adc ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_wdt include/arch/wdt.hpp src/wdt.cpp)
target_link_libraries(avr_wdt PUBLIC tos::core)
tos_install(avr_wdt ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(avr_drivers src/stub.cpp include/arch/drivers.hpp)
target_link_libraries(avr_drivers PUBLIC
        avr_gpio
        avr_usart
        avr_spi
        avr_timer
        avr_eeprom
        avr_twim
        avr_wdt)
tos_install(avr_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(avr_drivers PROPERTIES
        EXPORT_NAME arch_drivers)
add_library(arch_drivers ALIAS avr_drivers)