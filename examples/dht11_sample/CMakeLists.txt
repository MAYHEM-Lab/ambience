
add_executable(dht11_sample main.cpp)
target_link_libraries(dht11_sample PUBLIC tos_core tos_print tos_dht22 arch_drivers cwpack)
