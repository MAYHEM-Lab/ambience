add_library(tos_shell src/shell.cpp src/command.cpp include/tos/shell/command.hpp include/tos/shell/detail/echoing_uart.hpp)
target_include_directories(tos_shell PUBLIC include)
target_link_libraries(tos_shell PUBLIC tos_print tos_util_core)