project(tos)

add_executable(simple_esp main.cpp)
target_link_libraries(simple_esp PUBLIC tos_core esp82_drivers tos_print emsha)
