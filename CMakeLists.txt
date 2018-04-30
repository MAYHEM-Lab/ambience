cmake_minimum_required(VERSION 3.5)

set(TOS ON)

if (TOS_ARDUINO)
    set(ARDUINO_CPU atmega328)
    set(CMAKE_TOOLCHAIN_FILE /opt/arduino-cmake/cmake/ArduinoToolchain.cmake)
elseif(TOS_AVR)
    set(AVR_CPU atmega328p)
    set(CMAKE_TOOLCHAIN_FILE cmake/AvrToolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DF_CPU=16000000UL")
elseif(TOS_ESP)
    set(CMAKE_TOOLCHAIN_FILE cmake/lx106Toolchain.cmake)
endif()

project(tos)

message(STATUS "${CMAKE_CXX_FLAGS}")

add_subdirectory(src)

if (TOS_ARDUINO)
    add_subdirectory(examples/cmake_example)
elseif(TOS_AVR)
    add_subdirectory(examples/gpio)
    add_subdirectory(examples/spi)
    add_subdirectory(examples/sd)
elseif(TOS_ESP)
    add_subdirectory(examples/esp)
elseif(TOS_X86)
    add_subdirectory(examples/simple_portable)
    add_subdirectory(apps/cspot_lite)
endif()

set(include_dest "include/tos")
set(lib_dest "lib/tos")
