#include "common.hpp"
#include <chrono>
#include <nimble/nimble_npl.h>
#include <tos/mutex.hpp>
#include <tos/ft.hpp>

using namespace tos;

extern "C" {
ble_npl_error_t ble_npl_mutex_init(struct ble_npl_mutex* mu) {
    mu->mutex_ptr = new recursive_mutex;
    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_mutex_pend(struct ble_npl_mutex* mu, ble_npl_time_t timeout) {
    if (timeout == BLE_NPL_TIME_FOREVER) {
        static_cast<recursive_mutex*>(mu->mutex_ptr)->lock();
        return BLE_NPL_OK;
    }

    if (timeout == 0) {
        return static_cast<recursive_mutex*>(mu->mutex_ptr)->try_lock() ? BLE_NPL_OK : BLE_NPL_TIMEOUT;
    }
    
    auto down_res = static_cast<recursive_mutex*>(mu->mutex_ptr)
                        ->lock(*tos::nimble::alarm, std::chrono::milliseconds(timeout));
    if (down_res == tos::sem_ret::normal) {
        return BLE_NPL_OK;
    }
    return BLE_NPL_TIMEOUT;
}

ble_npl_error_t ble_npl_mutex_release(struct ble_npl_mutex* mu) {
    static_cast<recursive_mutex*>(mu->mutex_ptr)->unlock();
    return BLE_NPL_OK;
}
}