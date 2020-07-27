if (NOT "${HAL_NAME}" MATCHES "stm32f7")
    return()
endif()

add_library(tos_driver_stm32f7_ltdc include/tos/periph/stm32f7_ltdc.hpp stm32f7_ltdc.cpp)
target_link_libraries(tos_driver_stm32f7_ltdc tos::drivers)
target_include_directories(tos_driver_stm32f7_ltdc PUBLIC include)