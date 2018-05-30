project(tos CXX ASM)

add_library(avr_impl avr_impl.cpp tos_arch.hpp)
target_include_directories(avr_impl PRIVATE "../../")
target_include_directories(avr_impl PUBLIC ".")
target_compile_features(avr_impl PUBLIC cxx_std_14)
target_link_libraries(avr_impl PUBLIC tos_core)
