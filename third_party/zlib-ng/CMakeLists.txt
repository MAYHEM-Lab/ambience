set(ZLIB_ENABLE_TESTS OFF)
set(WITH_SANITIZER OFF)
set(HAVE_PTRDIFF_T ON CACHE BOOL "")
add_subdirectory(src)

target_link_libraries(zlib PUBLIC tos::libc)
target_include_directories(zlib PUBLIC src)
target_include_directories(zlib PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/src)