
add_executable(aws_push main.cpp axl_adapters.cpp resources.hpp priv_key.cpp rem_cert.cpp me_cert.cpp)
target_link_libraries(aws_push PUBLIC c tos_core esp82_drivers tos_print paho_mqtt posix_shim lwip_sntp)
target_compile_definitions(aws_push PUBLIC MQTT_DEBUG)