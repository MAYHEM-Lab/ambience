add_executable(ina219_demo main.cpp)
target_link_libraries(ina219_demo PUBLIC tos::core tos_ina219 arch_drivers tos::print)