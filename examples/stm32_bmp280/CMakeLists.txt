
add_executable(stm32_bmp280 main.cpp)
target_link_libraries(stm32_bmp280 tos_core stm32_drivers tos_bmp280 tos_print)