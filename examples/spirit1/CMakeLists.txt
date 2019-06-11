add_executable(spirit1 main.cpp)
add_subdirectory(SPIRIT1_Library)
target_link_libraries(spirit1 PUBLIC tos::core stm32_drivers tos::print spirit1_driver)

