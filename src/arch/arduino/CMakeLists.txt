
generate_arduino_library(arduino_impl
        SRCS arduino_impl.cpp
        BOARD uno)

target_compile_definitions(arduino_impl PUBLIC ARDUINO_CMAKE)
target_include_directories(arduino_impl PRIVATE "../../")
target_compile_features(arduino_impl PUBLIC cxx_std_14)