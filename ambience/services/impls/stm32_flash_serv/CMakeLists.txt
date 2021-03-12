if (NOT ${TOS_PLATFORM} MATCHES "stm32")
    return()
endif()

add_library(stm32_flash_serv impl.cpp)
target_link_libraries(stm32_flash_serv PUBLIC block_memory_schema stm32_drivers)