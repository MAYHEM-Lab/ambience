project(tos_core CXX C ASM)

add_library(stm32_hal_core
        include/tos_arch.hpp
        include/tos_platform.hpp
        external/CMSIS/Device/ST/STM32F7xx/Source/Templates/system_stm32f7xx.c
        external/CMSIS/Device/ST/STM32F7xx/Source/Templates/gcc/startup_stm32f746xx.s

        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_gpio.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_tim_ex.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_cortex.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_pwr_ex.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_rcc_ex.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_uart.c
        external/STM32F7xx_HAL_Driver/Src/stm32f7xx_hal_usart.c
        core.cpp)

target_include_directories(stm32_hal_core SYSTEM PUBLIC "external/CMSIS/Device/ST/STM32F7xx/Include")
target_include_directories(stm32_hal_core SYSTEM PUBLIC "external/CMSIS/Include")
target_include_directories(stm32_hal_core SYSTEM PUBLIC "external/STM32F7xx_HAL_Driver/Inc")
target_compile_definitions(stm32_hal_core PUBLIC STM32F746xx)

target_link_libraries(stm32_hal_core PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}/ld")
target_link_libraries(stm32_hal_core PUBLIC "-T${TOS_LD_FILE}")

target_link_libraries(stm32_hal_core PUBLIC "-specs=nano.specs -specs=nosys.specs -Wl,--start-group -lgcc -lm -lc -lg -lstdc++ -lsupc++ -Wl,--end-group -Wl,--undefined=main")
target_compile_features(stm32_hal_core PUBLIC cxx_std_17)
target_link_libraries(stm32_hal_core PUBLIC tos_util_core)

tos_install(stm32_hal_core ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(stm32_hal_impl main.cpp)
target_link_libraries(stm32_hal_impl PUBLIC stm32_hal_core)
target_link_libraries(stm32_hal_impl PUBLIC tos_ft)
