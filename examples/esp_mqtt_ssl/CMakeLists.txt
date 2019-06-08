
add_executable(esp_mqtt_ssl main.cpp)
target_link_libraries(esp_mqtt_ssl PUBLIC /home/fatih/bearssl-esp8266/esp8266/libbearssl.a)
target_link_libraries(esp_mqtt_ssl PUBLIC tos_core
        esp82_drivers tos_print paho_mqtt
        cwpack tos_adxl345 lwip_sntp)
target_compile_definitions(esp_mqtt_ssl PUBLIC MQTT_DEBUG)
target_include_directories(esp_mqtt_ssl PUBLIC "/home/fatih/bearssl-esp8266/inc")
