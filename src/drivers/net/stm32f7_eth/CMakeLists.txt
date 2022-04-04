if (NOT "${HAL_NAME}" MATCHES "stm32f7")
    return()
endif()

add_library(stm32f7_eth driver.cpp)
target_link_libraries(stm32f7_eth PUBLIC tos_drivers stm32_hal_core lwipcore)
target_include_directories(stm32f7_eth PUBLIC include)
