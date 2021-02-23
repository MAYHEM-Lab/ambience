add_library(lfs
        src/lfs.c
        src/lfs.h
        src/lfs_util.c
        src/lfs_util.h)
target_include_directories(lfs PUBLIC src)
target_link_libraries(lfs PUBLIC tos::libc)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_compile_definitions(lfs PUBLIC LFS_YES_TRACE)
endif()
