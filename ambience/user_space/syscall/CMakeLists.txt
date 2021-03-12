if (NOT IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${TOS_ARCH}")
    message(STATUS "ae user space does not exist on ${TOS_ARCH}")
    return()
endif()

add_subdirectory(${TOS_ARCH})

target_include_directories(ae_user_space PRIVATE include)