
add_executable(avr_lcd main.cpp)
target_link_libraries(avr_lcd tos::core tos::print tos_lcd avr_drivers)