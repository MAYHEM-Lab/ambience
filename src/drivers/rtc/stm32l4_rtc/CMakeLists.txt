if (NOT "${HAL_NAME}" MATCHES "stm32l4")
    return()
endif()

add_library(stm32l4_rtc stm32l4_rtc.cpp include/tos/periph/stm32l4_rtc.hpp)
target_link_libraries(stm32l4_rtc PUBLIC tos_drivers stm32_hal_core)
target_include_directories(stm32l4_rtc PUBLIC include)
