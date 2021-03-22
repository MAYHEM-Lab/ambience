add_library(ae_user_space
        user_space.cpp
        include/tos/ae/user_space.hpp
        entry.cpp
        syscall/include/tos/ae/detail/syscall.hpp)
target_link_libraries(ae_user_space PUBLIC tos_coro tos_util_core aecommon tos_arch_core)
target_link_libraries(ae_user_space PRIVATE tos_mem)

target_include_directories(ae_user_space PUBLIC include)
target_link_libraries(ae_user_space PUBLIC "-L ${CMAKE_CURRENT_SOURCE_DIR}/ld")

add_subdirectory(syscall)

add_executable(sample_group sample.cpp)
target_link_libraries(sample_group PRIVATE ae_user_space)
target_link_libraries(sample_group PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/ld/01.ld")

add_executable(sample_group2 sample.cpp)
target_link_libraries(sample_group2 PRIVATE ae_user_space)
target_link_libraries(sample_group2 PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/ld/02.ld")
