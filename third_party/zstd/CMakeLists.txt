set(ZSTD_MULTITHREAD_SUPPORT OFF)
set(ZSTD_BUILD_PROGRAMS OFF)
set(ZSTD_BUILD_TESTS OFF)
set(ZSTD_BUILD_SHARED OFF)
set(ZSTD_BUILD_SHATIC ON)

add_subdirectory(src/build/cmake)

target_link_libraries(libzstd_static PUBLIC tos::libc)