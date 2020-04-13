add_executable(avr_adxl345 main.cpp)
target_link_libraries(avr_adxl345 PUBLIC tos::core tos_adxl345 avr_drivers tos::print)