project(tos)

if (TOS_AVR)
    add_subdirectory(arch/avr)
elseif(TOS_ESP)
    add_subdirectory(arch/lx106)
elseif(TOS_X86)
    add_subdirectory(arch/x86)
elseif(TOS_ARM)
    add_subdirectory(arch/nrf52)
elseif(TOS_STM32)
	add_subdirectory(arch/stm32)
endif()

add_library(tos_drivers common/driver_base.hpp src/stub.cpp)
target_link_libraries(tos_drivers PUBLIC tos::core tos::util)
target_include_directories(tos_drivers PUBLIC .)
add_library(tos::drivers ALIAS tos_drivers)

add_library(tos_spi common/spi.hpp src/stub.cpp common/timer.hpp src/spi.cpp)
target_link_libraries(tos_spi PUBLIC tos_gpio tos_core)
target_include_directories(tos_spi PUBLIC .)

add_library(tos_usart common/usart.hpp src/stub.cpp)
target_link_libraries(tos_usart PUBLIC tos::core)
target_include_directories(tos_usart PUBLIC .)

add_library(tos_gpio common/gpio.hpp src/stub.cpp)
target_include_directories(tos_gpio PUBLIC .)

add_library(tos_dht22 common/dht22.hpp src/dht22.cpp)
target_link_libraries(tos_dht22 PUBLIC tos_core tos_gpio)
target_include_directories(tos_dht22 PUBLIC .)

add_library(tos_hx711 common/hx711.hpp src/stub.cpp)
target_link_libraries(tos_hx711 PUBLIC tos::core tos_gpio)
target_include_directories(tos_hx711 PUBLIC .)

add_library(tos::hx711 ALIAS tos_hx711)

add_library(tos_xbee
        common/xbee.hpp
        common/xbee/constants.hpp
        common/xbee/types.hpp
        common/xbee/request.hpp
        common/xbee/response.hpp
        src/stub.cpp common/xbee/utility.hpp)
target_link_libraries(tos_xbee PUBLIC tos_core tos_usart Boost::sml)
target_include_directories(tos_xbee PUBLIC .)

if (TOS_AVR)
    add_library(tos_spi_sd src/spi_sd.cpp common/sd/spi_sd.hpp common/sd/sd_info.hpp)
    target_link_libraries(tos_spi_sd PUBLIC tos_core tos_spi tos_gpio)
    target_include_directories(tos_spi_sd PRIVATE .)

    add_library(tos_nrf24 common/nrf24.hpp src/nrf24.cpp)
    target_link_libraries(tos_nrf24 PUBLIC tos_core)
    target_include_directories(tos_nrf24 PUBLIC .)
endif()

add_library(tos_alarm common/alarm.hpp src/stub.cpp)
target_link_libraries(tos_alarm PUBLIC tos_core)
target_include_directories(tos_alarm PUBLIC .)

add_library(tos_eeprom common/eeprom.hpp src/stub.cpp)
target_link_libraries(tos_eeprom PUBLIC tos_core)
target_include_directories(tos_eeprom PUBLIC .)

add_library(tos_tty common/tty.hpp src/stub.cpp)
target_include_directories(tos_tty PUBLIC .)

add_library(tos_i2c common/i2c.hpp src/stub.cpp)
target_include_directories(tos_i2c PUBLIC .)

add_library(tos_lcd common/lcd.hpp src/stub.cpp)
target_include_directories(tos_lcd PUBLIC .)

add_library(tos_ina219 common/ina219.hpp src/stub.cpp common/detail/ina219_constants.hpp)
target_link_libraries(tos_ina219 PUBLIC tos_i2c)
add_library(tos::ina219 ALIAS tos_ina219)

if (TOS_ESP)
    add_library(tos_adxl345 common/adxl345.hpp src/adxl345.cpp)
    target_link_libraries(tos_adxl345 PUBLIC tos_core)
    target_include_directories(tos_adxl345 PUBLIC .)
endif()

add_library(tos_inet common/inet/tcp_ip.hpp common/inet/tcp_stream.hpp src/stub.cpp common/inet/lwip.hpp)
target_include_directories(tos_inet PUBLIC .)
