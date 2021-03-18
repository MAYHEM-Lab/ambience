add_library(aecommon src/common.cpp include/tos/ae/entry.hpp)
target_include_directories(aecommon PUBLIC include)
target_link_libraries(aecommon PUBLIC tos_util_core)

add_subdirectory(boot)
add_subdirectory(services)
add_subdirectory(user_space)
add_subdirectory(kernel_space)