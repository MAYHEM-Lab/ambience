add_library(iot_epd_board INTERFACE)
target_link_libraries(iot_epd_board INTERFACE arch_drivers l4_iot_disco_board)
target_include_directories(iot_epd_board INTERFACE "..")