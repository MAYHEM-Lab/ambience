add_executable(aekernel
    include/tos/ae/kernel/syscall.hpp
    include/tos/ae/kernel/user_group.hpp
    include/tos/ae/kernel/rings.hpp
    include/tos/ae/kernel/address_space.hpp)
target_link_libraries(aekernel PUBLIC tos_util_core aecommon tos_core tos_dynamic_log)
target_include_directories(aekernel PUBLIC include)

add_subdirectory(common)
add_subdirectory(platform)