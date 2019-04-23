
add_executable(esp_mqtt main.cpp fake_accel.hpp axl_adapters.cpp)
target_link_libraries(esp_mqtt PUBLIC c tos_core lwip_sntp esp82_drivers tos_print paho_mqtt cwpack posix_shim tos_adxl345 lwip_sntp)
target_compile_definitions(esp_mqtt PUBLIC MQTT_DEBUG)
