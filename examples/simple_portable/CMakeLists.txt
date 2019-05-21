project(tos)

add_executable(simple_portable main.cpp)
target_link_libraries(simple_portable
        tos_core
        tos_print
        tos_tty
        arch_drivers)