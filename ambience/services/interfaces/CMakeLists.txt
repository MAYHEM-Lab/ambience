add_lidlc(echo_schema echo.lidl)

add_lidlc(calc_schema calc.lidl)

add_lidlc(block_memory_schema block_memory.lidl)

add_lidlc(temp_sensor_schema temp_sensor.yaml)

add_lidlc(i2c_schema i2c.lidl)
add_lidl_dependency(i2c_schema tos_i2c_schema)

add_lidlc(group_schema group.lidl)

add_lidlc(alarm_schema alarm.lidl)
add_lidlc(filesystem_schema file_system.lidl)
target_include_directories(filesystem_schema INTERFACE .)

add_lidlc(machine_schema machine.lidl)

add_lidlc(import_export_schema import_export.lidl)

add_lidlc(agent_schema agent.lidl)

add_lidlc(kvstore_schema kvstore.lidl)

add_lidlc(networking_schema networking.lidl)

add_lidlc(weather_sensor_schema weather_sensor.lidl)

add_lidlc(random_schema random.lidl)