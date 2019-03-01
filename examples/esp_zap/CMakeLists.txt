add_executable(esp_zap main.cpp)
target_link_libraries(esp_zap PUBLIC tos_core esp82_drivers tos_print)
target_compile_features(esp_zap PUBLIC cxx_std_17)