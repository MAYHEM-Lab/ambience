if (NOT ${TOS_CPU} MATCHES "l4" AND NOT ${TOS_CPU} MATCHES "f7")
    return()
endif()

add_executable(mpu_demo main.cpp)
target_link_libraries(mpu_demo PUBLIC arch_drivers tos::core tos::print tos_dynamic_log)
