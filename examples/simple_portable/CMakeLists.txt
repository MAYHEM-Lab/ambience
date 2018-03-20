project(tos)

add_executable(simple_portable main.cpp)
target_link_libraries(simple_portable PUBLIC tos_core)