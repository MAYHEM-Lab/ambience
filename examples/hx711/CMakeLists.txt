
add_executable(hx711_demo hx711_demo.cpp)
target_link_libraries(hx711_demo PUBLIC tos::core stm32_drivers tos::print)