add_library(aekernellib
    include/tos/ae/kernel/syscall.hpp
    include/tos/ae/kernel/user_group.hpp
    include/tos/ae/kernel/rings.hpp
    include/tos/ae/kernel/address_space.hpp stub.cpp)
target_link_libraries(aekernellib PUBLIC aecommon)
target_include_directories(aekernellib PUBLIC include)

add_executable(aekernel "")
target_link_libraries(aekernel PUBLIC tos_util_core tos_core tos_dynamic_log aekernellib)

add_subdirectory(common)
add_subdirectory(platform)