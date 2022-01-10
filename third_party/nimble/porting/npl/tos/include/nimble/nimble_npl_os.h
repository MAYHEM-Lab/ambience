#pragma once

#include <stdint.h>

#define BLE_NPL_OS_ALIGNMENT    4

#define BLE_NPL_TIME_FOREVER    (unsigned)-1

#ifdef __cplusplus
extern "C" {
#endif

/* This should be compatible with TickType_t */
typedef uint32_t ble_npl_time_t;
typedef int32_t ble_npl_stime_t;

struct ble_npl_event {
    #define BLE_NPL_EVENT_SIZE (4 * sizeof(void*))
    _Alignas(_Alignof(void*)) char buffer[BLE_NPL_EVENT_SIZE];
    uint32_t initd;
};

struct ble_npl_eventq {
    #define BLE_NPL_EVENTQ_SIZE (5 * sizeof(void*))
    _Alignas(_Alignof(void*)) char buffer[BLE_NPL_EVENTQ_SIZE];
    uint32_t initd;
};

struct ble_npl_callout {
    #define BLE_NPL_CALLOUT_SIZE (6 * sizeof(void*))
    _Alignas(_Alignof(void*)) char buffer[BLE_NPL_CALLOUT_SIZE];

    ble_npl_time_t ticks;
    struct ble_npl_eventq *evq;
    struct ble_npl_event ev;
    uint32_t initd;
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
