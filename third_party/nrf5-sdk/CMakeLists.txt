if (NOT ${TOS_CORE} MATCHES "nrf5")
    return()
endif()

add_library(nrf_cc310 INTERFACE)
target_link_libraries(nrf_cc310 INTERFACE external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.12.a)
target_include_directories(nrf_cc310 SYSTEM INTERFACE external/nrf_cc310/include)

add_library(nrf5-sdk-prs modules/nrfx/drivers/src/prs/nrfx_prs.c)
target_compile_definitions(nrf5-sdk-prs PUBLIC NRFX_PRS_ENABLED=1 NRFX_PRS_BOX_0_ENABLED=1)
target_link_libraries(nrf5-sdk-prs PUBLIC nrf5-sdk)

add_library(nrf5-sdk-usart
    modules/nrfx/drivers/src/nrfx_uarte.c
    modules/nrfx/drivers/src/nrfx_uart.c)
target_link_libraries(nrf5-sdk-usart PUBLIC nrf5-sdk)
target_compile_definitions(nrf5-sdk-usart PUBLIC 
    NRFX_UARTE_ENABLED=1 
    NRFX_UARTE0_ENABLED=1 
    UART_LEGACY_SUPPORT=0 
    NRFX_UART_ENABLED=0)

add_library(nrf5-sdk-timer
    modules/nrfx/drivers/src/nrfx_timer.c)
target_link_libraries(nrf5-sdk-timer PUBLIC nrf5-sdk)
target_compile_definitions(nrf5-sdk-timer PUBLIC
    NRFX_TIMER_ENABLED=1
    NRFX_TIMER0_ENABLED=1
    NRFX_TIMER1_ENABLED=1
    NRFX_TIMER2_ENABLED=1
    TIMER_LEGACY_SUPPORT=0)


add_library(nrf5-sdk-twim
    modules/nrfx/drivers/src/nrfx_twim.c)
target_link_libraries(nrf5-sdk-twim PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-twim PUBLIC
    NRFX_TWIM_ENABLED=1
    NRFX_TWIM0_ENABLED=1
    TWIM_LEGACY_SUPPORT=0)

add_library(nrf5-sdk-spim
    modules/nrfx/drivers/src/nrfx_spim.c)
target_link_libraries(nrf5-sdk-spim PUBLIC nrf5-sdk-prs)
target_compile_definitions(nrf5-sdk-spim PUBLIC
        NRFX_SPIM_ENABLED=1
        NRFX_SPIM0_ENABLED=1
        SPIM_LEGACY_SUPPORT=0)

add_library(nrf5-sdk-drivers INTERFACE)
target_link_libraries(nrf5-sdk-drivers INTERFACE 
    nrf5-sdk-prs
    nrf5-sdk-usart
    nrf5-sdk-timer
    nrf5-sdk-twim
    nrf5-sdk-spim
)

add_library(nrf5-sdk 
    modules/nrfx/mdk/system_nrf52.c
    components/ble/ble_advertising/ble_advertising.c
    components/ble/nrf_ble_gatt/nrf_ble_gatt.c
    components/ble/nrf_ble_qwr/nrf_ble_qwr.c
    components/ble/common/ble_advdata.c
    components/ble/common/ble_conn_params.c
    components/ble/common/ble_conn_state.c
    components/ble/common/ble_srv_common.c
    components/libraries/util/app_error.c
    components/libraries/util/app_error_handler_gcc.c
    components/libraries/util/app_error_weak.c
    components/ble/ble_services/ble_nus/ble_nus.c
    components/softdevice/common/nrf_sdh_ble.c
    components/softdevice/common/nrf_sdh.c
    components/libraries/experimental_section_vars/nrf_section_iter.c
    components/libraries/timer/app_timer.c
    components/libraries/util/app_util_platform.c
    components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c
    components/libraries/atomic_flags/nrf_atflags.c
    components/libraries/atomic/nrf_atomic.c
    components/libraries/sha256/sha256.c
)

target_include_directories(nrf5-sdk PUBLIC include)
target_include_directories(nrf5-sdk SYSTEM PUBLIC integration/nrfx)

target_include_directories(nrf5-sdk SYSTEM PUBLIC modules/nrfx)
target_include_directories(nrf5-sdk SYSTEM PUBLIC modules/nrfx/mdk/)
target_include_directories(nrf5-sdk SYSTEM PUBLIC modules/nrfx/hal)
target_include_directories(nrf5-sdk SYSTEM PUBLIC modules/nrfx/drivers/include)

target_include_directories(nrf5-sdk SYSTEM PUBLIC components/toolchain/cmsis/include/)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/delay)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/log)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/timer)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/atomic)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/atomic_flags)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/strerror)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/log/src)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/experimental_section_vars)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/libraries/util)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/common)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/ble_link_ctx_manager)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/nrf_ble_gatt)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/ble_advertising)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/nrf_ble_qwr)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/ble/ble_services/ble_nus)
#target_include_directories(nrf5-sdk ${SDK_ PUBLICRnts/drivers_nrf/nrf_soc_nosd)
message(STATUS ${NRF52_SOFTDEV})
if (${NRF52_SOFTDEV} MATCHES "S140")
    target_include_directories(nrf5-sdk SYSTEM PUBLIC components/softdevice/s140/headers)
endif()
if (${NRF52_SOFTDEV} MATCHES "S132")
    target_include_directories(nrf5-sdk SYSTEM PUBLIC components/softdevice/s132/headers)
endif()
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/softdevice/mbr/headers)
target_include_directories(nrf5-sdk SYSTEM PUBLIC components/softdevice/common)
