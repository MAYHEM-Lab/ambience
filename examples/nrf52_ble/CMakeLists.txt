add_executable(nrf52_ble_demo main.cpp gatt.cpp)
target_link_libraries(nrf52_ble_demo PUBLIC tos_core tos_print tos_board tos_dynamic_log nimble etl)
