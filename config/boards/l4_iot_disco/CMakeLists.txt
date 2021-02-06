add_library(l4_iot_disco_board INTERFACE)
target_link_libraries(l4_iot_disco_board INTERFACE arch_drivers)
target_include_directories(l4_iot_disco_board INTERFACE "..")