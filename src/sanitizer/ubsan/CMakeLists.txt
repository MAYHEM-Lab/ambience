add_library(ubsan_support handlers.cpp include/tos/sanitizer/ubsan.hpp)
target_include_directories(ubsan_support PUBLIC include)
target_link_libraries(ubsan_support PUBLIC tos_util_core)