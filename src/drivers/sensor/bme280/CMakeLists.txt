add_library(tos_bme280
        bme280/bme280.c
        include/tos/device/bme280/bme280.h
        include/tos/device/bme280/bme280_defs.h
        include/tos/device/bme280.hpp bme280.cpp)
target_link_libraries(tos_bme280 PUBLIC tos_util_core tos_drivers tos_i2c weather_sensor_schema)
target_include_directories(tos_bme280 PUBLIC include)
target_compile_features(tos_bme280 PUBLIC cxx_std_14)
target_compile_definitions(tos_bme280 PUBLIC BME280_FLOAT_ENABLE)
