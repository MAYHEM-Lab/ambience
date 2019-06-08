add_executable(ism43362 main.cpp)
target_link_libraries(ism43362 PUBLIC tos::core stm32_drivers tos::print)