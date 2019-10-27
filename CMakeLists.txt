cmake_minimum_required(VERSION 3.5)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(config/HandleConfig.cmake)

if (TOS_TOOLCHAIN)
    set(CMAKE_TOOLCHAIN_FILE cmake/${TOS_TOOLCHAIN}.cmake)
endif()

unset(WIN32)
project(tos VERSION 0.1.0)

include(Tos)

if (NOT ${CMAKE_BUILD_TYPE} MATCHES "MinSizeRel")
    message(WARNING "Build is not optimized for minimum size\nThis is not an error, but binaries will be large.")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" CACHE STRING "CXXFLAGS")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}" CACHE STRING "LINKERFLAGS")

message(STATUS "${CMAKE_CXX_FLAGS}")

enable_testing()

add_subdirectory(src)
add_subdirectory(libs)

SUBDIRLIST(APP_DIRS "${CMAKE_CURRENT_LIST_DIR}/apps")

message(STATUS ${APP_DIRS})

foreach(app ${APP_DIRS})
    add_subdirectory("apps/${app}")
endforeach()

add_subdirectory(examples/backtrace)
if(${TOS_CORE} MATCHES "atmega")
    add_subdirectory(examples/gpio)
    add_subdirectory(examples/spi)
    add_subdirectory(examples/alarm)
    add_subdirectory(examples/sd)
    add_subdirectory(examples/temp)
    add_subdirectory(examples/int)
    add_subdirectory(examples/simple_portable)
    add_subdirectory(examples/nrf24)
    add_subdirectory(examples/xbee)
    add_subdirectory(examples/ina219)
    add_subdirectory(examples/avr_adxl345)
    add_subdirectory(examples/avr_lcd)
elseif(${TOS_CORE} MATCHES "esp")
    if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/libs/mqtt")
        add_subdirectory(examples/esp_mqtt)
        add_subdirectory(examples/esp_mqtt_ssl)
    endif()

    add_subdirectory(examples/esp_lwip)
    add_subdirectory(examples/esp_conn)
    #add_subdirectory(examples/esp_ssl)
    add_subdirectory(examples/esp_spi)
    add_subdirectory(examples/esp_adc)
    #add_subdirectory(examples/bearssl)

    #add_subdirectory(examples/aws_push)

    add_subdirectory(examples/dht11_sample)
    add_subdirectory(examples/esp_udp)

    add_subdirectory(examples/capabilities)

    add_subdirectory(examples/esp_adxl345)
    add_subdirectory(examples/esp_ssd1306)
elseif(${TOS_CORE} MATCHES "x86_hosted")
    #add_subdirectory(examples/bearssl)
    add_subdirectory(examples/simple_portable)
    add_subdirectory(examples/x86_timer)
    #add_subdirectory(examples/capabilities)

    add_subdirectory(tests)
elseif(${TOS_CORE} MATCHES "nrf52")
    add_subdirectory(examples/nrf52)
elseif(${TOS_CORE} MATCHES "stm32")
    add_subdirectory(examples/stm32_blink)
    add_subdirectory(examples/photon)
    add_subdirectory(examples/stm32_dht22)
    add_subdirectory(examples/hx711)
    add_subdirectory(examples/stm32_i2c_lcd)
    add_subdirectory(examples/stm32_lora)
    add_subdirectory(examples/stm32_hm10)
    add_subdirectory(examples/stm32_bmp280)
    add_subdirectory(examples/ssd1306)
    add_subdirectory(examples/bme280)
    add_subdirectory(examples/rfid)
    add_subdirectory(examples/ism43362)
    add_subdirectory(examples/spirit1)
    add_subdirectory(examples/spbtle-rf)
    add_subdirectory(examples/epd)
endif()
