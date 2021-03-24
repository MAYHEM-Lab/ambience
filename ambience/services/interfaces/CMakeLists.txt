add_lidlc(calc_schema calc.yaml)

add_lidlc(block_memory_schema block_memory.yaml)

add_lidlc(temp_sensor_schema temp_sensor.yaml)

add_lidlc(i2c_schema i2c.lidl)
target_link_libraries(i2c_schema INTERFACE tos_i2c_schema)