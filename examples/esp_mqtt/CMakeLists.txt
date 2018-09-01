
add_executable(esp_mqtt main.cpp sntp.c sntp.h time.h fake_accel.hpp axl_adapters.cpp)
target_link_libraries(esp_mqtt PUBLIC c tos_core esp82_drivers tos_print paho_mqtt umsgpack posix_shim tos_adxl345)
target_compile_definitions(esp_mqtt PUBLIC MQTT_DEBUG LWIP_DEBUG)
