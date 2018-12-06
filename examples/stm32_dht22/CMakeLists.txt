
add_executable(stm32_dht22 main.cpp)
target_link_libraries(stm32_dht22 PUBLIC tos::core stm32_drivers tos::print)