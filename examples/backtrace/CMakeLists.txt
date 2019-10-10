add_executable(backtrace main.cpp)
target_link_libraries(backtrace PUBLIC tos_core arch_drivers tos_print)