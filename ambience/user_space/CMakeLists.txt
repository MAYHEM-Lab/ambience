if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/syscall/${TOS_ARCH}")
    message(STATUS "ae user space does not exist on ${TOS_ARCH}")
    return()
endif()

add_library(ae_user_space
        user_space.cpp
        include/tos/ae/user_space.hpp
        entry.cpp
        syscall/include/tos/ae/detail/syscall.hpp)
target_link_libraries(ae_user_space PUBLIC tos_coro tos_util_core aecommon tos_arch_core log_schema)
target_link_libraries(ae_user_space PRIVATE tos_mem)

target_include_directories(ae_user_space PUBLIC include)

if (NOT ${TOS_ARCH} MATCHES "wasm")
    target_link_libraries(ae_user_space PUBLIC "-L ${CMAKE_CURRENT_SOURCE_DIR}/ld")
else()
    target_link_libraries(ae_user_space PUBLIC "-e _user_code")
endif()

add_subdirectory(syscall)

add_executable(sample_group sample.cpp)
target_link_libraries(sample_group PRIVATE ae_user_space)

if (NOT ${TOS_ARCH} MATCHES "wasm")
    target_link_libraries(sample_group PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/ld/01.ld")
endif()

target_link_libraries(sample_group PRIVATE basic_calc)

add_executable(sample_group2 sample.cpp)
target_link_libraries(sample_group2 PRIVATE ae_user_space)
target_link_libraries(sample_group2 PRIVATE basic_calc)

if (NOT ${TOS_ARCH} MATCHES "wasm")
    target_link_libraries(sample_group2 PUBLIC "-T ${CMAKE_CURRENT_SOURCE_DIR}/ld/02.ld")
endif()
