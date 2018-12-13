
add_executable(stm32_lora main.cpp)
target_link_libraries(stm32_lora PUBLIC tos::core stm32_drivers tos::print tos::rn2903)