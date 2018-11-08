
add_executable(esp_adc main.cpp)
target_link_libraries(esp_adc PUBLIC tos::core esp82_drivers tos::print)