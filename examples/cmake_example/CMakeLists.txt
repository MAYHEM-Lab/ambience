cmake_minimum_required(VERSION 2.8)

project(example C CXX)

set(example_BOARD uno)

set(example_SRCS main.cpp)

set(example_PORT /dev/ttyUSB1)

generate_arduino_firmware(example
        LIBS tos_core
        ARDLIBS LowPower)

target_compile_features(example PUBLIC cxx_std_14)