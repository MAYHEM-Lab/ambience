project(tos CXX ASM)

add_library(x86_impl x86_impl.cpp set_stack.s arch_impl.hpp)
target_include_directories(x86_impl PRIVATE "../../")
target_compile_features(x86_impl PUBLIC cxx_std_14)
