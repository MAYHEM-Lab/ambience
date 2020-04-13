add_executable(esp_adxl345 main.cpp)
target_link_libraries(esp_adxl345 PUBLIC tos::core tos_adxl345 esp82_drivers tos::print cwpack boost_pfr)