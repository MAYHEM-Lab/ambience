add_library(nucleo_l412_board INTERFACE)
target_link_libraries(nucleo_l412_board INTERFACE arch_drivers)
target_include_directories(nucleo_l412_board INTERFACE "..")