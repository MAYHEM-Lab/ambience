
add_executable(xbee_tx tx_main.cpp)
target_link_libraries(xbee_tx PUBLIC tos_core tos_xbee arch_drivers tos_print)