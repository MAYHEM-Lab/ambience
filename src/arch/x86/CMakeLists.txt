project(tos CXX ASM)

add_library(x86_impl x86_impl.cpp)
target_link_libraries(x86_impl PUBLIC x86_core)

add_library(x86_core x86_core.cpp set_stack.s include/tos_arch.hpp)
target_compile_options(x86_core PUBLIC "-fsanitize=undefined")
target_compile_options(x86_core PUBLIC "-U_FORTIFY_SOURCE")
target_compile_features(x86_core PUBLIC cxx_std_14)
target_link_libraries(x86_core PUBLIC tos_ft ubsan)

find_package(Boost REQUIRED system)
target_link_libraries(x86_impl PUBLIC Boost::boost Boost::system pthread)

tos_install(x86_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(x86_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
