add_service(bme280_service temp_sensor impl.cpp)
target_link_libraries(bme280_service PUBLIC tos_bme280 i2c_schema)