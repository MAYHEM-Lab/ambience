add_executable(blink main.cpp)
target_link_libraries(blink PUBLIC tos::core tos::print tos_board)