add_library(tos_dump dump.cpp include/tos/dump.hpp)
target_link_libraries(tos_dump PUBLIC tos_util_core)
target_include_directories(tos_dump PUBLIC include)