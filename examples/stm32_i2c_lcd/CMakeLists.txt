
add_executable(stm32_i2c_lcd main.cpp)
target_link_libraries(stm32_i2c_lcd PUBLIC tos::core stm32_drivers tos::print)