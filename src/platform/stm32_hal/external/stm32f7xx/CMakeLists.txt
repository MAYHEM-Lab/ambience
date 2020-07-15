add_library(stm32_hal_drivers
        drivers/src/${HAL_NAME}_hal.c
        drivers/src/${HAL_NAME}_hal_gpio.c
        drivers/src/${HAL_NAME}_hal_tim.c
        drivers/src/${HAL_NAME}_hal_tim_ex.c
        drivers/src/${HAL_NAME}_hal_cortex.c
        drivers/src/${HAL_NAME}_hal_pwr.c
        drivers/src/${HAL_NAME}_hal_pwr_ex.c
        drivers/src/${HAL_NAME}_hal_rcc.c
        drivers/src/${HAL_NAME}_hal_rcc_ex.c
        drivers/src/${HAL_NAME}_hal_uart.c
        drivers/src/${HAL_NAME}_hal_uart_ex.c
        drivers/src/${HAL_NAME}_hal_usart.c
        drivers/src/${HAL_NAME}_hal_dma.c
        drivers/src/${HAL_NAME}_hal_spi.c
        #drivers/src/${HAL_NAME}_hal_spi_ex.c
        drivers/src/${HAL_NAME}_ll_exti.c
        drivers/src/${HAL_NAME}_hal_rtc.c
        drivers/src/${HAL_NAME}_hal_rtc_ex.c
        drivers/src/${HAL_NAME}_hal_flash.c
        drivers/src/${HAL_NAME}_hal_flash_ex.c
        drivers/src/${HAL_NAME}_hal_i2c.c
        drivers/src/${HAL_NAME}_hal_i2c_ex.c
        drivers/src/${HAL_NAME}_hal_sdram.c
        drivers/src/${HAL_NAME}_hal_ltdc.c
        drivers/src/${HAL_NAME}_ll_fmc.c
        drivers/src/${HAL_NAME}_hal_dma2d.c
        )
target_include_directories(stm32_hal_drivers PUBLIC drivers/include)
target_include_directories(stm32_hal_drivers PUBLIC soc)
target_compile_definitions(stm32_hal_drivers PUBLIC ${CPU_NAME} USE_FULL_LL_DRIVER)
target_link_libraries(stm32_hal_drivers PUBLIC cmsis::core)