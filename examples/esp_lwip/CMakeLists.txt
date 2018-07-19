
add_executable(esp_lwip main.cpp)
target_link_libraries(esp_lwip PUBLIC tos_core esp82_drivers tos_print)