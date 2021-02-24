if (${TOS_NO_LIBC})
    add_library(tos_libc empty.cpp)
    add_library(tos::libc ALIAS tos_libc)
    return()
endif()

project(tos C CXX ASM)

add_subdirectory(src/newlib)

add_library(tos::libc ALIAS picolibc)