add_executable(clock_demo main.cpp)
target_link_libraries(clock_demo PUBLIC tos_core arch_drivers tos_print)