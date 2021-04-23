target_include_directories(picolibc SYSTEM PUBLIC include)
target_include_directories(picolibc SYSTEM PUBLIC tinystdio)

add_subdirectory(ctype)
add_subdirectory(errno)
add_subdirectory(stdlib)
add_subdirectory(string)
add_subdirectory(tinystdio)

add_subdirectory(machine)
