add_executable(bme280_demo main.cpp)
target_link_libraries(bme280_demo PUBLIC tos::core stm32_drivers tos_bme280 tos::print)