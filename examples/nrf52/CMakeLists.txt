add_executable(nrf52_blink main.cpp)
target_link_libraries(nrf52_blink PUBLIC tos_core tos_print frozen tos_board)
