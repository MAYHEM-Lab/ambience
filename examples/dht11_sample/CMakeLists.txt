
add_executable(dht11_sample main.cpp)
target_link_libraries(dht11_sample tos_core tos_print tos_dht22 esp82_drivers cwpack)