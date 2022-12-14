project(tos_core CXX C ASM)

string(TOLOWER ${CPU_NAME} LOWER_CPU_NAME)

add_subdirectory(external)
add_subdirectory(external/${HAL_NAME})

add_library(stm32_hal_core
        core.cpp
        include/tos/platform.hpp
        include/stm32_hal/dma.hpp
        include/stm32_hal/hal.hpp
        include/stm32_hal/gpio.hpp
        include/stm32_hal/rcc.hpp
        include/stm32_hal/rcc_ex.hpp
        include/stm32_hal/usart.hpp
        include/stm32_hal/tim.hpp
        include/stm32_hal/spi.hpp
        include/stm32_hal/ll_exti.hpp
        include/stm32_hal/rtc.hpp
        include/stm32_hal/rng.hpp
)

target_include_directories(stm32_hal_core SYSTEM PUBLIC "external/${HAL_NAME}/soc")
target_include_directories(stm32_hal_core SYSTEM PUBLIC "external/${HAL_NAME}/drivers/include")
target_compile_definitions(stm32_hal_core PUBLIC STM32_HAL_NAME=${HAL_NAME})
target_link_libraries(stm32_hal_core PUBLIC tos_util_core Boost::preprocessor)
target_link_libraries(stm32_hal_core PUBLIC stm32_hal_drivers cmsis::core arch_arm)

tos_install(stm32_hal_core ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_hal_impl
        main.cpp
        external/${HAL_NAME}/soc/system_${HAL_NAME}.c
)
target_link_libraries(stm32_hal_impl PUBLIC stm32_hal_core)
target_link_libraries(stm32_hal_impl PUBLIC tos_jobs)
set_target_properties(stm32_hal_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/${TOS_LD_FILE})
target_link_libraries(stm32_hal_impl PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}/ld")
target_link_libraries(stm32_hal_impl PUBLIC "-T${TOS_LD_FILE}")

add_subdirectory(startup)
