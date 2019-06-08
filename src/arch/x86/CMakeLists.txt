project(tos CXX ASM)

add_library(x86_impl x86_impl.cpp)
target_link_libraries(x86_impl PUBLIC x86_core)

add_library(x86_core x86_core.cpp include/tos_arch.hpp)
# without this flag, the runtime detects us jumping into the middle of a stack
target_compile_options(x86_core PUBLIC "-U_FORTIFY_SOURCE")
target_compile_features(x86_core PUBLIC cxx_std_14)
target_link_libraries(x86_core PUBLIC tos_ft)

# we use boost.asio for asynchronous operations
find_package(Boost REQUIRED system)
target_link_libraries(x86_impl PUBLIC Boost::boost Boost::system pthread)

tos_install(x86_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
tos_install(x86_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
