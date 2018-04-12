generate_arduino_library(arduino_impl
        SRCS LowPower/LowPower.cpp arduino_impl.cpp tos_arch.hpp
        BOARD nano)

target_compile_definitions(arduino_impl PUBLIC ARDUINO_CMAKE)
target_include_directories(arduino_impl PRIVATE "../../")
target_include_directories(arduino_impl PUBLIC ".")
target_compile_features(arduino_impl PUBLIC cxx_std_14)