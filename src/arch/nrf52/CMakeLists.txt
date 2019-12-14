project(tos CXX ASM)

add_library(nrf_cc310 INTERFACE)
target_link_libraries(nrf_cc310 INTERFACE ${SDK_ROOT}/external/nrf_cc310/lib/cortex-m4/hard-float/libnrf_cc310_0.9.12.a)
target_include_directories(nrf_cc310 SYSTEM INTERFACE ${SDK_ROOT}/external/nrf_cc310/include)

add_library(nrf52_core nrf52_core.cpp include/tos_arch.hpp)
tos_install(nrf52_core ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_core PUBLIC tos_util_core)

add_library(nrf52_impl nrf52_impl.cpp include/sdk_config.h startup.S
        ${SDK_ROOT}/modules/nrfx/mdk/system_nrf52.c
        ${SDK_ROOT}/components/ble/ble_advertising/ble_advertising.c
        ${SDK_ROOT}/components/ble/nrf_ble_gatt/nrf_ble_gatt.c
        ${SDK_ROOT}/components/ble/nrf_ble_qwr/nrf_ble_qwr.c
        ${SDK_ROOT}/components/ble/common/ble_advdata.c
        ${SDK_ROOT}/components/ble/common/ble_conn_params.c
        ${SDK_ROOT}/components/ble/common/ble_conn_state.c
        ${SDK_ROOT}/components/ble/common/ble_srv_common.c
        ${SDK_ROOT}/components/libraries/util/app_error.c
        ${SDK_ROOT}/components/libraries/util/app_error_handler_gcc.c
        ${SDK_ROOT}/components/libraries/util/app_error_weak.c
        ${SDK_ROOT}/components/ble/ble_services/ble_nus/ble_nus.c
        ${SDK_ROOT}/components/softdevice/common/nrf_sdh_ble.c
        ${SDK_ROOT}/components/softdevice/common/nrf_sdh.c
        ${SDK_ROOT}/components/libraries/experimental_section_vars/nrf_section_iter.c
        ${SDK_ROOT}/components/libraries/timer/app_timer.c
        ${SDK_ROOT}/components/libraries/util/app_util_platform.c
        ${SDK_ROOT}/components/ble/ble_link_ctx_manager/ble_link_ctx_manager.c
        ${SDK_ROOT}/components/libraries/atomic_flags/nrf_atflags.c
        ${SDK_ROOT}/components/libraries/atomic/nrf_atomic.c
        ${SDK_ROOT}/components/libraries/sha256/sha256.c
        )
tos_install(nrf52_impl ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(nrf52_impl PUBLIC nrf52_core)
target_compile_features(nrf52_impl PUBLIC cxx_std_17)

target_compile_definitions(nrf52_impl PUBLIC __HEAP_SIZE=8097 __STACK_SIZE=2048)

target_link_libraries(nrf52_impl PUBLIC tos_core)
target_link_libraries(nrf52_impl PUBLIC "--specs=nano.specs -lc -lnosys -lm")

target_link_libraries(nrf52_impl PUBLIC "-L${CMAKE_CURRENT_SOURCE_DIR}/ld")
target_link_libraries(nrf52_impl PUBLIC "-T${TOS_LD_FILE}")

set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/${TOS_LD_FILE})
set_target_properties(nrf52_impl PROPERTIES LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ld/nrf5x_common.ld)
