cmake_minimum_required(VERSION 3.5)

set(CMAKE_TOOLCHAIN_FILE /opt/arduino-cmake/cmake/ArduinoToolchain.cmake)

project(tos)

add_subdirectory(src)
add_subdirectory(examples/cmake_example)