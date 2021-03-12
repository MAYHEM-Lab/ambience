if (NOT ${TOS_PLATFORM} MATCHES "stm32")
    return()
endif()

add_service(stm32_i2c_serv i2c impl.cpp)
target_link_libraries(stm32_i2c_serv PUBLIC stm32_drivers tos_i2c)