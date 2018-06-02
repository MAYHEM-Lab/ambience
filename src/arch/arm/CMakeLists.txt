project(tos CXX ASM)

add_library(arm_impl arm_impl.cpp tos_arch.hpp)
target_include_directories(arm_impl PRIVATE "../../")
target_include_directories(arm_impl PUBLIC ".")
target_compile_options(arm_impl PUBLIC "-std=c++14")
target_link_libraries(arm_impl PUBLIC tos_core)
