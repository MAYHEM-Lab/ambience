
add_library(x86_stdio stdio.hpp src/stdio.cpp)
target_include_directories(x86_stdio PUBLIC .)
target_link_libraries(x86_stdio PRIVATE tos_core)
