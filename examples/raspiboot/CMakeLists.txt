add_executable(raspiboot main.cpp)
target_link_libraries(raspiboot PUBLIC tos_core arch_drivers tos_print)