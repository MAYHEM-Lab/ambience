
add_executable(stm32_hm10 hm10_main.cpp)
target_link_libraries(stm32_hm10 PUBLIC tos::core stm32_drivers tos::hm10 tos::print)