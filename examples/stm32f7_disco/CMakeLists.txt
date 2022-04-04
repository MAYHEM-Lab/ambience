add_executable(stm32f7_disco_demo main.cpp)
target_link_libraries(stm32f7_disco_demo PUBLIC tos::core stm32_drivers tos::print
        tos_driver_stm32f7_sdram tos_driver_stm32f7_ltdc tos_driver_stm32f7_dma2d
        tos_board stm32f7_eth tos_dynamic_log)