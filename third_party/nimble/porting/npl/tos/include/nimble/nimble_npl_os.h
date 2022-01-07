#pragma once

#include <stdint.h>

#define BLE_NPL_OS_ALIGNMENT    4

#define BLE_NPL_TIME_FOREVER    -1

#ifdef __cplusplus
extern "C" {
#endif

/* This should be compatible with TickType_t */
typedef uint32_t ble_npl_time_t;
typedef int32_t ble_npl_stime_t;

struct ble_npl_event {
    _Alignas(_Alignof(void*)) char buffer[4 * sizeof(void*)];
};

struct ble_npl_eventq {
    _Alignas(_Alignof(void*)) char buffer[4 * sizeof(void*)];
};

struct ble_npl_callout {
    // TimerHandle_t handle;
    // struct ble_npl_eventq *evq;
    // struct ble_npl_event ev;
};

struct ble_npl_mutex {
    void* mutex_ptr;
};

struct ble_npl_sem {
    void* sem_ptr;
};

#ifdef __cplusplus
}
#endif
