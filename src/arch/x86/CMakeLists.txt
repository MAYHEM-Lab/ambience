project(tos CXX ASM)

add_library(x86_impl x86_impl.cpp set_stack.s include/tos_arch.hpp)
tos_install(x86_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(x86_impl PUBLIC "-fsanitize=undefined")
target_compile_options(x86_impl PUBLIC "-U_FORTIFY_SOURCE")
target_compile_features(x86_impl PUBLIC cxx_std_14)
target_link_libraries(x86_impl PRIVATE tos_ft ubsan)

find_package(Boost REQUIRED system)
target_link_libraries(x86_impl PUBLIC Boost::boost Boost::system pthread)