add_library(toblerone_board INTERFACE)
target_link_libraries(toblerone_board INTERFACE arch_drivers)
target_include_directories(toblerone_board INTERFACE "..")