
add_executable(blink main.cpp)
target_link_libraries(blink PUBLIC tos::core stm32_drivers tos::print nlohmann::json)