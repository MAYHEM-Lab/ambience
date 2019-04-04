project(tos)

if (TOS_AVR)
    add_subdirectory(arch/avr)
elseif(TOS_ESP)
    add_subdirectory(arch/lx106)
elseif(TOS_X86)
    add_subdirectory(arch/x86)
elseif(TOS_NRF52)
    add_subdirectory(arch/nrf52)
elseif(TOS_STM32)
	add_subdirectory(arch/stm32)
endif()

add_library(tos_drivers include/common/driver_base.hpp src/stub.cpp)
target_link_libraries(tos_drivers PUBLIC tos::core tos::util)
tos_install(tos_drivers ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(tos::drivers ALIAS tos_drivers)

add_library(tos_spi include/common/spi.hpp src/stub.cpp include/common/timer.hpp src/spi.cpp)
target_link_libraries(tos_spi PUBLIC tos_gpio tos_core)
tos_install(tos_spi ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_usart include/common/usart.hpp src/stub.cpp)
target_link_libraries(tos_usart PUBLIC tos::core)
tos_install(tos_usart ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_gpio include/common/gpio.hpp src/stub.cpp)
tos_install(tos_gpio ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_dht22 include/common/dht22.hpp src/dht22.cpp)
target_link_libraries(tos_dht22 PUBLIC tos_core tos_gpio)
tos_install(tos_dht22 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_hx711 include/common/hx711.hpp src/stub.cpp)
target_link_libraries(tos_hx711 PUBLIC tos::core tos_gpio)
tos_install(tos_hx711 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos::hx711 ALIAS tos_hx711)

add_library(tos_xbee
        include/common/xbee.hpp
        include/common/xbee/constants.hpp
        include/common/xbee/types.hpp
        include/common/xbee/request.hpp
        include/common/xbee/response.hpp
        src/stub.cpp
        include/common/xbee/utility.hpp)
target_link_libraries(tos_xbee PUBLIC tos_core tos_usart Boost::sml)
tos_install(tos_xbee ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (TOS_AVR)
    add_library(tos_spi_sd src/spi_sd.cpp include/common/sd/spi_sd.hpp include/common/sd/sd_info.hpp)
    target_link_libraries(tos_spi_sd PUBLIC tos_core tos_spi tos_gpio)
    target_include_directories(tos_spi_sd PRIVATE .)

    add_library(tos_nrf24 include/common/nrf24.hpp src/nrf24.cpp)
    target_link_libraries(tos_nrf24 PUBLIC tos_core)
    tos_install(tos_nrf24 ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

add_library(tos_alarm include/common/alarm.hpp src/stub.cpp)
target_link_libraries(tos_alarm PUBLIC tos_util_core)
tos_install(tos_alarm ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_eeprom include/common/eeprom.hpp src/stub.cpp)
target_link_libraries(tos_eeprom PUBLIC tos_core)
tos_install(tos_eeprom ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_tty include/common/tty.hpp src/stub.cpp)
tos_install(tos_tty ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_i2c include/common/i2c.hpp src/stub.cpp)
tos_install(tos_i2c ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_lcd include/common/lcd.hpp src/stub.cpp include/common/detail/lcd_constants.hpp)
tos_install(tos_lcd ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_ina219 include/common/ina219.hpp src/stub.cpp include/common/detail/ina219_constants.hpp)
target_link_libraries(tos_ina219 PUBLIC tos_i2c)
add_library(tos::ina219 ALIAS tos_ina219)
tos_install(tos_ina219 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_rn2903
        include/common/rn2903/mac.hpp
        src/stub.cpp
        include/common/rn2903/sys.hpp
        include/common/rn2903/rn2903.hpp
        include/common/rn2903/rn2903_common.hpp)
target_link_libraries(tos_rn2903 PUBLIC tos_usart tos::print)
add_library(tos::rn2903 ALIAS tos_rn2903)
tos_install(tos_rn2903 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_hm10 include/common/hm-10.hpp src/stub.cpp)
target_link_libraries(tos_hm10 PUBLIC tos_usart tos::print)
add_library(tos::hm10 ALIAS tos_hm10)
tos_install(tos_hm10 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_adxl345 include/common/adxl345.hpp src/adxl345.cpp)
target_link_libraries(tos_adxl345 PUBLIC tos_core tos_i2c)
tos_install(tos_adxl345 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_bmp280 include/common/bmp280.hpp src/bmp280.cpp)
target_link_libraries(tos_bmp280 PUBLIC tos_core tos_i2c)
tos_install(tos_bmp280 ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(tos_bme280 src/bme280/bme280.c include/common/bme280/bme280.h include/common/bme280/bme280_defs.h include/common/bme280.hpp)
target_link_libraries(tos_bme280 PUBLIC tos_core tos_i2c)
tos_install(tos_bme280 ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(tos_bme280 PUBLIC cxx_std_14)
target_compile_definitions(tos_bme280 PUBLIC BME280_FLOAT_ENABLE)


add_library(tos_inet
        include/common/inet/tcp_ip.hpp
        include/common/inet/tcp_stream.hpp
        src/stub.cpp
        include/common/inet/lwip.hpp)
tos_install(tos_inet ${CMAKE_CURRENT_SOURCE_DIR}/include)
