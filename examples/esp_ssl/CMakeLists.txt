
add_executable(esp_ssl main.cpp sntp.c sntp.h time.h)
target_link_libraries(esp_ssl PUBLIC tos_core esp82_drivers tos_print)