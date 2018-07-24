
add_executable(esp_conn main.cpp)
target_link_libraries(esp_conn PUBLIC tos_core esp82_drivers tos_print)