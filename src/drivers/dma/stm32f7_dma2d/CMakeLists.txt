if (NOT "${HAL_NAME}" MATCHES "stm32f7")
    return()
endif()

add_library(tos_driver_stm32f7_dma2d include/tos/periph/stm32f7_dma2d.hpp stm32f7_dma2d.cpp)
target_link_libraries(tos_driver_stm32f7_dma2d tos::drivers tos_gfx2)
target_include_directories(tos_driver_stm32f7_dma2d PUBLIC include)