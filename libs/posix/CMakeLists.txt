
add_library(posix_shim src/syscalls.cpp include/tos/posix.hpp)
target_link_libraries(posix_shim PUBLIC tos_core)
target_include_directories(posix_shim PUBLIC include)