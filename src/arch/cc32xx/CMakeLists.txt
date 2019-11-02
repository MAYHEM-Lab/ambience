set(TI_SDK_ROOT /opt/ti/simplelink_cc32xx_sdk_3_30_01_02/)

add_library(ti_sdk "")
target_sources(ti_sdk PRIVATE
        ${TI_SDK_ROOT}/kernel/nortos/startup/startup_cc32xx_gcc.c
        ${TI_SDK_ROOT}/kernel/nortos/NoRTOS.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/ClockPSysTick_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/DebugP_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/HwiPCC32XX_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/MutexP_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/posix_sleep.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/PowerCC32XX_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/QueueP_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/SemaphoreP_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/SwiP_nortos.c
        ${TI_SDK_ROOT}/kernel/nortos/dpl/SystemP_nortos.c
        ${TI_SDK_ROOT}/source/ti/drivers/power/PowerCC32XX.c
        )
target_include_directories(ti_sdk SYSTEM PUBLIC ${TI_SDK_ROOT}/source)
target_include_directories(ti_sdk SYSTEM PUBLIC ${TI_SDK_ROOT}/kernel/nortos)

add_library(ti_driverlib "")
target_sources(ti_driverlib PRIVATE
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/adc.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/aes.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/camera.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/cpu.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/crc.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/des.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/flash.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/gpio.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/hwspinlock.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/i2c.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/interrupt.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/i2s.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/pin.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/prcm.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/sdhost.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/shamd5.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/spi.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/systick.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/timer.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/uart.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/udma.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/utils.c
        ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/wdt.c
    )
target_link_libraries(ti_driverlib PUBLIC ti_sdk ${TI_SDK_ROOT}/source/ti/devices/cc32xx/driverlib/gcc/Release/driverlib.a)
target_include_directories(ti_driverlib PUBLIC ${TI_SDK_ROOT}/source/ti/devices/cc32xx)

add_library(cc32xx_core "")
target_sources(cc32xx_core PRIVATE core.cpp)
target_link_libraries(cc32xx_core PUBLIC ti_sdk tos_util_core cmsis::core)
target_include_directories(cc32xx_core PUBLIC include)
target_link_libraries(cc32xx_core PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(cc32xx_core PUBLIC "-Tcc3235sf.ld")
target_link_libraries(cc32xx_core PUBLIC "-specs=nano.specs -specs=nosys.specs -Wl,--start-group -lgcc -lm -lc -lg -lstdc++ -lsupc++ -Wl,--end-group -Wl,--undefined=main")
target_compile_features(cc32xx_core PUBLIC cxx_std_17)

add_library(cc32xx_impl "")
target_sources(cc32xx_impl PRIVATE impl.cpp)
target_link_libraries(cc32xx_impl PUBLIC tos_ft ti_driverlib ti_sdk cc32xx_core)
target_include_directories(cc32xx_impl PUBLIC include)

set_target_properties(cc32xx_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/cc3235sf.ld)