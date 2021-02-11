if (NOT ${TOS_PLATFORM} MATCHES "nrf5")
    return()
endif ()

add_library(nrf_cc310 INTERFACE)
target_link_libraries(nrf_cc310 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.12.a)
target_include_directories(nrf_cc310 SYSTEM INTERFACE src/external/nrf_cc310/include)

add_library(nrf5-sdk-prs src/modules/nrfx/drivers/src/prs/nrfx_prs.c)
target_compile_definitions(nrf5-sdk-prs PUBLIC NRFX_PRS_ENABLED=1 NRFX_PRS_BOX_0_ENABLED=1)
target_link_libraries(nrf5-sdk-prs PUBLIC nrf5-sdk)

add_library(nrf5-sdk-usart
        src/modules/nrfx/drivers/src/nrfx_uarte.c
        src/modules/nrfx/drivers/src/nrfx_uart.c)
target_link_libraries(nrf5-sdk-usart PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-usart PUBLIC
        NRFX_UARTE_ENABLED=1
        NRFX_UARTE0_ENABLED=1
        UART_LEGACY_SUPPORT=0
        NRFX_UART_ENABLED=0)

add_library(nrf5-sdk-timer
        src/modules/nrfx/drivers/src/nrfx_timer.c)
target_link_libraries(nrf5-sdk-timer PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-timer PUBLIC
        NRFX_TIMER_ENABLED=1
        NRFX_TIMER0_ENABLED=1
        NRFX_TIMER1_ENABLED=1
        NRFX_TIMER2_ENABLED=1
        TIMER_LEGACY_SUPPORT=0)


add_library(nrf5-sdk-twim
        src/modules/nrfx/drivers/src/nrfx_twim.c)
target_link_libraries(nrf5-sdk-twim PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-twim PUBLIC
        NRFX_TWIM_ENABLED=1
        NRFX_TWIM0_ENABLED=1
        TWIM_LEGACY_SUPPORT=0)

add_library(nrf5-sdk-spim
        src/modules/nrfx/drivers/src/nrfx_spim.c)
target_link_libraries(nrf5-sdk-spim PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-spim PUBLIC
        NRFX_SPIM_ENABLED=1
        NRFX_SPIM0_ENABLED=1
        SPIM_LEGACY_SUPPORT=0)

add_library(nrf5-sdk-nvmc
        src/modules/nrfx/drivers/src/nrfx_nvmc.c)
target_link_libraries(nrf5-sdk-nvmc PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-nvmc PUBLIC
        NRFX_NVMC_ENABLED=1)

add_library(nrf5-sdk-drivers INTERFACE)
target_link_libraries(nrf5-sdk-drivers INTERFACE
        nrf5-sdk-prs
        nrf5-sdk-usart
        nrf5-sdk-timer
        nrf5-sdk-twim
        nrf5-sdk-spim
        nrf5-sdk-nvmc
        )

add_library(nrf5-sdk
        src/modules/nrfx/mdk/system_nrf52.c
        src/components/ble/ble_advertising/ble_advertising.c
        src/components/ble/nrf_ble_gatt/nrf_ble_gatt.c
        src/components/ble/nrf_ble_qwr/nrf_ble_qwr.c
        src/components/ble/common/ble_advdata.c
        src/components/ble/common/ble_conn_params.c
        src/components/ble/common/ble_conn_state.c
        src/components/ble/common/ble_srv_common.c
        src/components/libraries/util/app_error.c
        src/components/libraries/util/app_error_handler_gcc.c
        src/components/libraries/util/app_error_weak.c
        src/components/ble/ble_services/ble_nus/ble_nus.c
        src/components/softdevice/common/nrf_sdh_ble.c
        src/components/softdevice/common/nrf_sdh.c
        src/components/libraries/experimental_section_vars/nrf_section_iter.c
        src/components/libraries/timer/app_timer.c
        src/components/libraries/util/app_util_platform.c
        src/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c
        src/components/libraries/atomic_flags/nrf_atflags.c
        src/components/libraries/atomic/nrf_atomic.c
        src/components/libraries/sha256/sha256.c
        src/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c)
target_link_libraries(nrf5-sdk PUBLIC cmsis::core picolibc)

target_include_directories(nrf5-sdk PUBLIC include)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/integration/nrfx)

target_include_directories(nrf5-sdk SYSTEM PUBLIC src/modules/nrfx)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/modules/nrfx/mdk/)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/modules/nrfx/hal)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/modules/nrfx/drivers/include)

target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/delay)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/log)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/timer)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/atomic)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/atomic_flags)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/strerror)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/log/src)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/experimental_section_vars)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/util)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/pwr_mgmt)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/libraries/mutex)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/common)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/ble_link_ctx_manager)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/nrf_ble_gatt)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/ble_advertising)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/nrf_ble_qwr)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/ble/ble_services/ble_nus)
#target_include_directories(nrf5-sdk ${SDK_ PUBLICRnts/drivers_nrf/nrf_soc_nosd)
message(STATUS ${NRF52_SOFTDEV})
if (${NRF52_SOFTDEV} MATCHES "S140")
    target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/softdevice/s140/headers)
endif ()
if (${NRF52_SOFTDEV} MATCHES "S132")
    target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/softdevice/s132/headers)
endif ()
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/softdevice/mbr/headers)
target_include_directories(nrf5-sdk SYSTEM PUBLIC src/components/softdevice/common)


add_library(cc310_benchmark cc310_benchmark.cpp)
target_link_libraries(cc310_benchmark PUBLIC nrf_cc310 ubench arch_drivers)
add_benchmark(cc310_benchmark)