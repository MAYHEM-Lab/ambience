add_library(nucleo_l476_board INTERFACE)
target_link_libraries(nucleo_l476_board INTERFACE arch_drivers)
target_include_directories(nucleo_l476_board INTERFACE "..")