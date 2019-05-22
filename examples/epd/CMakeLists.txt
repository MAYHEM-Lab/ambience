add_executable(epd main.cpp)
target_link_libraries(epd PUBLIC tos::core stm32_drivers tos::print)