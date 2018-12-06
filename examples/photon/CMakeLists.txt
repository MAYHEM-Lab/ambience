
add_executable(photon_blink main.cpp)
target_link_libraries(photon_blink PUBLIC tos::core stm32_drivers tos::print nlohmann::json)