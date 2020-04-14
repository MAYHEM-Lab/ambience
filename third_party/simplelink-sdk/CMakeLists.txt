if (NOT ${TOS_CORE} MATCHES "cc32xx")
    return()
endif()
add_library(ti_sdk "")
target_sources(ti_sdk PRIVATE
        kernel/nortos/startup/startup_cc32xx_gcc.c
        kernel/nortos/NoRTOS.c
        kernel/nortos/dpl/ClockPSysTick_nortos.c
        kernel/nortos/dpl/DebugP_nortos.c
        kernel/nortos/dpl/HwiPCC32XX_nortos.c
        kernel/nortos/dpl/MutexP_nortos.c
        kernel/nortos/dpl/posix_sleep.c
        kernel/nortos/dpl/PowerCC32XX_nortos.c
        kernel/nortos/dpl/QueueP_nortos.c
        kernel/nortos/dpl/SemaphoreP_nortos.c
        kernel/nortos/dpl/SwiP_nortos.c
        kernel/nortos/dpl/SystemP_nortos.c
        kernel/nortos/dpl/PowerCC32XX_nortos.c
        )
target_include_directories(ti_sdk SYSTEM PUBLIC source)
target_include_directories(ti_sdk SYSTEM PUBLIC kernel/nortos)

add_library(ti_driverlib "")
target_sources(ti_driverlib PRIVATE
        source/ti/devices/cc32xx/driverlib/adc.c
        source/ti/devices/cc32xx/driverlib/aes.c
        source/ti/devices/cc32xx/driverlib/camera.c
        source/ti/devices/cc32xx/driverlib/cpu.c
        source/ti/devices/cc32xx/driverlib/crc.c
        source/ti/devices/cc32xx/driverlib/des.c
        source/ti/devices/cc32xx/driverlib/flash.c
        source/ti/devices/cc32xx/driverlib/gpio.c
        source/ti/devices/cc32xx/driverlib/hwspinlock.c
        source/ti/devices/cc32xx/driverlib/i2c.c
        source/ti/devices/cc32xx/driverlib/interrupt.c
        source/ti/devices/cc32xx/driverlib/i2s.c
        source/ti/devices/cc32xx/driverlib/pin.c
        source/ti/devices/cc32xx/driverlib/prcm.c
        source/ti/devices/cc32xx/driverlib/sdhost.c
        source/ti/devices/cc32xx/driverlib/shamd5.c
        source/ti/devices/cc32xx/driverlib/spi.c
        source/ti/devices/cc32xx/driverlib/systick.c
        source/ti/devices/cc32xx/driverlib/timer.c
        source/ti/devices/cc32xx/driverlib/uart.c
        source/ti/devices/cc32xx/driverlib/udma.c
        source/ti/devices/cc32xx/driverlib/utils.c
        source/ti/devices/cc32xx/driverlib/wdt.c
        source/ti/devices/cc32xx/driverlib/shamd5.c
        source/ti/devices/cc32xx/driverlib/udma.c
        )
target_link_libraries(ti_driverlib PUBLIC ti_sdk ${CMAKE_CURRENT_SOURCE_DIR}/source/ti/devices/cc32xx/driverlib/gcc/Release/driverlib.a)
target_include_directories(ti_driverlib PUBLIC source/ti/devices/cc32xx)

add_library(ti_simplelink_src "")
target_sources(ti_simplelink_src PRIVATE
        source/ti/drivers/net/wifi/source/device.c
        source/ti/drivers/net/wifi/source/driver.c
        source/ti/drivers/net/wifi/source/flowcont.c
        source/ti/drivers/net/wifi/source/fs.c
        source/ti/drivers/net/wifi/source/netapp.c
        source/ti/drivers/net/wifi/source/netcfg.c
        source/ti/drivers/net/wifi/source/netutil.c
        source/ti/drivers/net/wifi/source/nonos.c
        source/ti/drivers/net/wifi/source/sl_socket.c
        source/ti/drivers/net/wifi/source/spawn.c
        source/ti/drivers/net/wifi/source/wlan.c
        source/ti/drivers/net/wifi/porting/cc_pal.c
        source/ti/drivers/net/wifi/eventreg.c
        source/ti/drivers/net/wifi/source/wlanconfig.c
        )
target_include_directories(ti_simplelink_src PUBLIC source/ti/posix/gcc)
target_include_directories(ti_simplelink_src PUBLIC source/ti/net/bsd/sys)
target_include_directories(ti_simplelink_src PUBLIC source)

target_compile_definitions(ti_simplelink_src PUBLIC SL_SUPPORT_IPV6 DISABLE_DEBUGGER_RECONNECT)

add_library(ti_simplelink INTERFACE)
target_link_libraries(ti_simplelink INTERFACE ti_sdk ti_driverlib ${CMAKE_CURRENT_SOURCE_DIR}/source/ti/drivers/net/wifi/gcc/nortos/simplelink.a ti_sdk)
target_include_directories(ti_simplelink INTERFACE source/ti/posix/gcc)

add_library(ti_drivers "")
target_sources(ti_drivers PRIVATE
        source/ti/drivers/UART.c
        source/ti/drivers/Timer.c
        source/ti/drivers/SPI.c
        source/ti/drivers/power/PowerCC32XX.c
        source/ti/boards/CC3220SF_LAUNCHXL/CC3220SF_LAUNCHXL.c)
target_include_directories(ti_drivers PUBLIC source/)
target_link_libraries(ti_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/source/ti/drivers/lib/drivers_cc32xx.am4g)
#target_link_libraries(ti_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/kernel/nortos/lib/nortos_cc32xx.am4g)
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=PowerCC32XX_initPolicy")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=PowerCC32XX_enterLPDS")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=UARTBusy")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=TimerIntEnable")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SHAMD5DataLengthSet")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SPI_Params_init")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=uDMAErrorStatusGet")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=MutexP_lock")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SimpleLinkFatalErrorEventHandler")
