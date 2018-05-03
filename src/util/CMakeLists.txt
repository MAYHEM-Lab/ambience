project(tos)

add_library(tos_util src/stub.cpp include/tos/intrusive_list.hpp include/tos/intrusive_ptr.hpp include/tos/ring_buf.hpp include/tos/expected.hpp)
target_include_directories(tos_util PUBLIC "include")
target_compile_options(tos_util PUBLIC "-std=c++14")