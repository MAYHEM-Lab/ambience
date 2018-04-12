project(tos CXX ASM)

add_library(x86_impl x86_impl.cpp set_stack.s tos_arch.hpp)
target_include_directories(x86_impl PRIVATE "../../")
target_include_directories(x86_impl PUBLIC ".")
target_compile_features(x86_impl PUBLIC cxx_std_14)
