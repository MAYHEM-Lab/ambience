if (${TOS_NO_LIBC})
    return()
endif()

project(tos C CXX ASM)

add_subdirectory(src/newlib)

add_library(tos::libc ALIAS picolibc)