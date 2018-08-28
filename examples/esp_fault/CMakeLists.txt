
add_executable(esp_fault main.cpp)
target_link_libraries(esp_fault PUBLIC c tos_core tos_print esp82_drivers)