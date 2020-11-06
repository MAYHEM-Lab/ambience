project(tos)

add_subdirectory(arch/${TOS_DRIVERS})

add_library(tos_drivers include/common/driver_base.hpp src/stub.cpp)
target_link_libraries(tos_drivers PUBLIC tos_util_core tos_arch_core tos_ft tos_debug)
target_include_directories(tos_drivers PUBLIC include)
add_library(tos::drivers ALIAS tos_drivers)

add_library(tos_spi include/common/spi.hpp src/stub.cpp include/common/timer.hpp src/spi.cpp)
target_link_libraries(tos_spi PUBLIC tos_gpio tos_core)
target_include_directories(tos_spi PUBLIC include)

add_library(tos_usart include/common/usart.hpp src/stub.cpp)
target_link_libraries(tos_usart PUBLIC tos::core)
target_include_directories(tos_usart PUBLIC include)

add_library(tos_gpio include/common/gpio.hpp src/stub.cpp include/common/detail/gpio_ports.hpp)
target_include_directories(tos_gpio PUBLIC include)

add_library(tos_dht22 include/common/dht22.hpp src/dht22.cpp)
target_link_libraries(tos_dht22 PUBLIC tos_core tos_gpio)
target_include_directories(tos_dht22 PUBLIC include)

add_library(tos_hx711 include/common/hx711.hpp src/stub.cpp)
target_link_libraries(tos_hx711 PUBLIC tos::core tos_gpio)
target_include_directories(tos_hx711 PUBLIC include)

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
target_include_directories(tos_xbee PUBLIC include)

add_library(tos_spi_sd src/spi_sd.cpp include/common/sd/spi_sd.hpp include/common/sd/sd_info.hpp)
target_link_libraries(tos_spi_sd PUBLIC tos_core tos_spi tos_gpio)
target_include_directories(tos_spi_sd PRIVATE .)

if (${TOS_PLATFORM} MATCHES "atmega")
    add_library(tos_nrf24 include/common/nrf24.hpp src/nrf24.cpp)
    target_link_libraries(tos_nrf24 PUBLIC tos_core)
    target_include_directories(tos_nrf24 PUBLIC include)
endif()

add_library(tos_alarm include/common/alarm.hpp src/stub.cpp)
target_link_libraries(tos_alarm PUBLIC tos_util_core)
target_include_directories(tos_alarm PUBLIC include)

add_library(tos_eeprom include/common/eeprom.hpp src/stub.cpp)
target_link_libraries(tos_eeprom PUBLIC tos_core)
target_include_directories(tos_eeprom PUBLIC include)

add_library(tos_tty include/common/tty.hpp src/stub.cpp)
target_include_directories(tos_tty PUBLIC include)

add_library(tos_i2c include/common/i2c.hpp src/stub.cpp)
target_include_directories(tos_i2c PUBLIC include)

add_library(tos_lcd include/common/lcd.hpp src/stub.cpp include/common/detail/lcd_constants.hpp)
target_include_directories(tos_lcd PUBLIC include)

add_library(tos_ina219 include/common/ina219.hpp src/stub.cpp include/common/detail/ina219_constants.hpp)
target_link_libraries(tos_ina219 PUBLIC tos_i2c)
add_library(tos::ina219 ALIAS tos_ina219)
target_include_directories(tos_ina219 PUBLIC include)

add_library(tos_rn2903
        include/common/rn2903/mac.hpp
        src/stub.cpp
        include/common/rn2903/sys.hpp
        include/common/rn2903/rn2903.hpp
        include/common/rn2903/rn2903_common.hpp)
target_link_libraries(tos_rn2903 PUBLIC tos_usart tos::print)
add_library(tos::rn2903 ALIAS tos_rn2903)
target_include_directories(tos_rn2903 PUBLIC include)

add_library(tos_hm10 include/common/hm-10.hpp src/stub.cpp)
target_link_libraries(tos_hm10 PUBLIC tos_usart tos::print)
add_library(tos::hm10 ALIAS tos_hm10)
target_include_directories(tos_hm10 PUBLIC include)

add_library(tos_adxl345 include/common/adxl345.hpp src/adxl345.cpp)
target_link_libraries(tos_adxl345 PUBLIC tos_core tos_i2c)
target_include_directories(tos_adxl345 PUBLIC include)

add_library(tos_bmp280 include/common/bmp280.hpp src/bmp280.cpp)
target_link_libraries(tos_bmp280 PUBLIC tos_core tos_i2c)
target_include_directories(tos_bmp280 PUBLIC include)

add_library(tos_device_waveshare29bw include/common/epd/waveshare/bw29.hpp src/stub.cpp)
target_link_libraries(tos_device_waveshare29bw PUBLIC tos_core tos_spi tos_gfx)
target_include_directories(tos_device_waveshare29bw PUBLIC include)

add_library(tos_ble
        include/common/ble/address.hpp
        src/stub.cpp
        include/common/ble/gatt.hpp)
target_link_libraries(tos_ble PUBLIC tos_core)
target_include_directories(tos_ble PUBLIC include)

add_library(tos_ssd1306 include/common/ssd1306/constants.hpp include/common/ssd1306.hpp src/stub.cpp)
target_link_libraries(tos_ssd1306 PUBLIC tos_core)
target_include_directories(tos_ssd1306 PUBLIC include)

add_library(tos_clock include/common/clock.hpp src/stub.cpp)
target_link_libraries(tos_clock PUBLIC tos_core)
target_include_directories(tos_clock PUBLIC include)

add_library(tos_inet
        include/common/inet/tcp_ip.hpp
        include/common/inet/tcp_stream.hpp
        src/stub.cpp
        include/common/inet/lwip.hpp)
target_include_directories(tos_inet PUBLIC include)

add_subdirectory(sensor)
add_subdirectory(ble)
add_subdirectory(clock)
add_subdirectory(display)
add_subdirectory(dma)
add_subdirectory(memory)
add_subdirectory(rtc)