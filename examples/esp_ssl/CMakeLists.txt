
add_executable(esp_ssl main.cpp)
target_link_libraries(esp_ssl PUBLIC tos_core esp82_drivers tos_print lwip_sntp)
target_include_directories(esp_ssl PUBLIC "/home/fatih/bearssl-esp8266/inc")
target_link_libraries(esp_ssl PUBLIC /home/fatih/bearssl-esp8266/esp8266/libbearssl.a)
