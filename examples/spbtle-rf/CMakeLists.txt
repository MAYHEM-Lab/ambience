add_executable(stm32ble main.cpp)
target_link_libraries(stm32ble PUBLIC tos_core arch_drivers spbtle-rf tos_dynamic_log)