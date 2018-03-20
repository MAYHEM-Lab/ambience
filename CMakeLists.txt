cmake_minimum_required(VERSION 3.5)

if (TOS_ARDUINO)
    set(CMAKE_TOOLCHAIN_FILE /opt/arduino-cmake/cmake/ArduinoToolchain.cmake)
endif()

project(tos)

add_subdirectory(src)

if (ARDUINO_FOUND)
    add_subdirectory(examples/cmake_example)
else()
    add_subdirectory(examples/simple_portable)
endif()