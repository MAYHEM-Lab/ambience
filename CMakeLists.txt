cmake_minimum_required(VERSION 3.5)

if (TOS_ARDUINO)
    set(ARDUINO_CPU atmega328)
    set(CMAKE_TOOLCHAIN_FILE /opt/arduino-cmake/cmake/ArduinoToolchain.cmake)
elseif(TOS_AVR)
    set(AVR_CPU atmega328p)
    set(CMAKE_TOOLCHAIN_FILE cmake/AvrToolchain.cmake)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DF_CPU=16000000UL")
endif()

project(tos)

message(STATUS "${CMAKE_CXX_FLAGS}")

add_subdirectory(src)

if (TOS_ARDUINO)
    add_subdirectory(examples/cmake_example)
else()
    add_subdirectory(examples/simple_portable)
    add_subdirectory(examples/gpio)
    add_subdirectory(examples/spi)
endif()