project(tos)

add_executable(master master.cpp)
target_link_libraries(master PUBLIC tos_core)

add_executable(slave slave.cpp)
target_link_libraries(slave PUBLIC tos_core)
