
add_executable(master master.cpp app.hpp)
target_link_libraries(master PUBLIC tos::core stm32_drivers tos::print tos_xbee tos_dht22 cwpack)