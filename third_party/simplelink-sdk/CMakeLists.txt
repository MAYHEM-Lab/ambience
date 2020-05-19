if (NOT ${TOS_PLATFORM} MATCHES "cc32xx")
    return()
endif()
add_library(ti_sdk "")
target_sources(ti_sdk PRIVATE
        src/kernel/nortos/startup/startup_cc32xx_gcc.c
        src/kernel/nortos/NoRTOS.c
        src/kernel/nortos/dpl/ClockPSysTick_nortos.c
        src/kernel/nortos/dpl/DebugP_nortos.c
        src/kernel/nortos/dpl/HwiPCC32XX_nortos.c
        src/kernel/nortos/dpl/MutexP_nortos.c
        src/kernel/nortos/dpl/posix_sleep.c
        src/kernel/nortos/dpl/PowerCC32XX_nortos.c
        src/kernel/nortos/dpl/QueueP_nortos.c
        src/kernel/nortos/dpl/SemaphoreP_nortos.c
        src/kernel/nortos/dpl/SwiP_nortos.c
        src/kernel/nortos/dpl/SystemP_nortos.c
        src/kernel/nortos/dpl/PowerCC32XX_nortos.c
        )
target_include_directories(ti_sdk SYSTEM PUBLIC src/source)
target_include_directories(ti_sdk SYSTEM PUBLIC src/kernel/nortos)

add_library(ti_driverlib "")
target_sources(ti_driverlib PRIVATE
        src/source/ti/devices/cc32xx/driverlib/adc.c
        src/source/ti/devices/cc32xx/driverlib/aes.c
        src/source/ti/devices/cc32xx/driverlib/camera.c
        src/source/ti/devices/cc32xx/driverlib/cpu.c
        src/source/ti/devices/cc32xx/driverlib/crc.c
        src/source/ti/devices/cc32xx/driverlib/des.c
        src/source/ti/devices/cc32xx/driverlib/flash.c
        src/source/ti/devices/cc32xx/driverlib/gpio.c
        src/source/ti/devices/cc32xx/driverlib/hwspinlock.c
        src/source/ti/devices/cc32xx/driverlib/i2c.c
        src/source/ti/devices/cc32xx/driverlib/interrupt.c
        src/source/ti/devices/cc32xx/driverlib/i2s.c
        src/source/ti/devices/cc32xx/driverlib/pin.c
        src/source/ti/devices/cc32xx/driverlib/prcm.c
        src/source/ti/devices/cc32xx/driverlib/sdhost.c
        src/source/ti/devices/cc32xx/driverlib/shamd5.c
        src/source/ti/devices/cc32xx/driverlib/spi.c
        src/source/ti/devices/cc32xx/driverlib/systick.c
        src/source/ti/devices/cc32xx/driverlib/timer.c
        src/source/ti/devices/cc32xx/driverlib/uart.c
        src/source/ti/devices/cc32xx/driverlib/udma.c
        src/source/ti/devices/cc32xx/driverlib/utils.c
        src/source/ti/devices/cc32xx/driverlib/wdt.c
        src/source/ti/devices/cc32xx/driverlib/shamd5.c
        src/source/ti/devices/cc32xx/driverlib/udma.c
        )
target_link_libraries(ti_driverlib PUBLIC ti_sdk ${CMAKE_CURRENT_SOURCE_DIR}/src/source/ti/devices/cc32xx/driverlib/gcc/Release/driverlib.a)
target_include_directories(ti_driverlib PUBLIC src/source/ti/devices/cc32xx)

add_library(ti_simplelink_src "")
target_sources(ti_simplelink_src PRIVATE
        src/source/ti/drivers/net/wifi/source/device.c
        src/source/ti/drivers/net/wifi/source/driver.c
        src/source/ti/drivers/net/wifi/source/flowcont.c
        src/source/ti/drivers/net/wifi/source/fs.c
        src/source/ti/drivers/net/wifi/source/netapp.c
        src/source/ti/drivers/net/wifi/source/netcfg.c
        src/source/ti/drivers/net/wifi/source/netutil.c
        src/source/ti/drivers/net/wifi/source/nonos.c
        src/source/ti/drivers/net/wifi/source/sl_socket.c
        src/source/ti/drivers/net/wifi/source/spawn.c
        src/source/ti/drivers/net/wifi/source/wlan.c
        src/source/ti/drivers/net/wifi/porting/cc_pal.c
        src/source/ti/drivers/net/wifi/eventreg.c
        src/source/ti/drivers/net/wifi/source/wlanconfig.c
        )
target_include_directories(ti_simplelink_src PUBLIC src/source/ti/posix/gcc)
target_include_directories(ti_simplelink_src PUBLIC src/source/ti/net/bsd/sys)
target_include_directories(ti_simplelink_src PUBLIC src/source)

target_compile_definitions(ti_simplelink_src PUBLIC SL_SUPPORT_IPV6 DISABLE_DEBUGGER_RECONNECT)

add_library(ti_simplelink INTERFACE)
target_link_libraries(ti_simplelink INTERFACE ti_sdk ti_driverlib ${CMAKE_CURRENT_SOURCE_DIR}/src/source/ti/drivers/net/wifi/gcc/nortos/simplelink.a ti_sdk)
target_include_directories(ti_simplelink INTERFACE src/source/ti/posix/gcc)

add_library(ti_drivers "")
target_sources(ti_drivers PRIVATE
        src/source/ti/drivers/UART.c
        src/source/ti/drivers/Timer.c
        src/source/ti/drivers/SPI.c
        src/source/ti/drivers/power/PowerCC32XX.c
        src/source/ti/drivers/crypto/CryptoCC32XX.c
#        src/source/ti/drivers/uart/UARTCC32XX.c
#        src/source/ti/drivers/spi/SPICC32XXDMA.c
#        src/source/ti/drivers/gpio/GPIOCC32XX.c
#        src/source/ti/drivers/utils/RingBuf.c
#        src/source/ti/drivers/utils/List.c
#        src/source/ti/drivers/dma/UDMACC32XX.c
#        src/source/ti/drivers/power/PowerCC32XX.c
        src/source/ti/boards/CC3220SF_LAUNCHXL/CC3220SF_LAUNCHXL.c
        )
target_include_directories(ti_drivers PUBLIC src/source/)
target_link_libraries(ti_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/source/ti/drivers/lib/drivers_cc32xx.am4g)
#target_link_libraries(ti_drivers PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/nortos/lib/nortos_cc32xx.am4g)
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=PowerCC32XX_initPolicy")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=PowerCC32XX_enterLPDS")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=UARTBusy")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=TimerIntEnable")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SHAMD5DataLengthSet")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SPI_Params_init")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=uDMAErrorStatusGet")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=MutexP_lock")
target_link_libraries(ti_drivers PUBLIC "-Wl,--undefined=SimpleLinkFatalErrorEventHandler")
