
add_executable(stm32_ssd1306 main.cpp)
target_link_libraries(stm32_ssd1306 PUBLIC tos::core stm32_drivers tos::print)