add_executable(esp_ssd1306 main.cpp)
target_link_libraries(esp_ssd1306 PUBLIC tos::core tos_ssd1306 tos_gfx esp82_drivers tos::print)