project(tos)

add_executable(simple_portable main.cpp)
target_link_libraries(simple_portable PUBLIC tos_core tos_print x86_stdio tos_tty)