add_library(hosted_board INTERFACE)
target_link_libraries(hosted_board INTERFACE arch_drivers)
target_include_directories(hosted_board INTERFACE "..")