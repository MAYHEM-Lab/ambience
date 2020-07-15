if (NOT ${HAL_NAME} MATCH "stm32f7")
    return()
endif()

add_library(tos_driver_stm32f7_sdram include/tos/periph/stm32f7_sdram.hpp stm32f7_sdram.cpp)
target_link_libraries(tos_driver_stm32f7_sdram tos::drivers)
target_include_directories(tos_driver_stm32f7_sdram PUBLIC include)