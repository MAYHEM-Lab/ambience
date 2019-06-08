add_executable(stm32_adc_temp main.cpp)
target_link_libraries(stm32_adc_temp PUBLIC tos::core stm32_drivers tos::print)