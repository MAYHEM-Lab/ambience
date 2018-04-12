cmake_minimum_required(VERSION 2.8)

project(example C CXX)

set(example_BOARD nano)

set(example_SRCS main.cpp)

set(example_PORT /dev/cu.wchusbserial1410)

generate_arduino_firmware(example
        LIBS tos_core)

target_compile_features(example PUBLIC cxx_std_14)