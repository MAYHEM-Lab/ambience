
add_library(x86_stdio stdio.hpp src/stdio.cpp)
target_include_directories(x86_stdio PUBLIC .)
target_compile_options(x86_stdio PUBLIC -std=c++14)
target_link_libraries(x86_stdio PUBLIC tos_core tos_util)

add_library(x86_drivers drivers.hpp src/stub.cpp)
target_link_libraries(x86_drivers PUBLIC x86_stdio)