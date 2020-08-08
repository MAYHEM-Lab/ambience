include(lidlc)

add_lidlc(ble_service_schema service.yaml)
add_executable(stm32ble main.cpp)
target_link_libraries(stm32ble PUBLIC tos_core arch_drivers tos_spbtlerf tos_dynamic_log ble_service_schema)