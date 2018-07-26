
add_executable(esp_mqtt main.cpp)
target_link_libraries(esp_mqtt PUBLIC tos_core esp82_drivers tos_print paho_mqtt)
target_compile_definitions(esp_mqtt PUBLIC MQTTCLIENT_QOS2)