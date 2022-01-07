#include <nimble/nimble_npl.h>
#include <tos/mutex.hpp>

using namespace tos;

extern "C" {
ble_npl_error_t ble_npl_mutex_init(struct ble_npl_mutex* mu) {
    mu->mutex_ptr = new mutex;
    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_mutex_pend(struct ble_npl_mutex* mu, ble_npl_time_t timeout) {
    static_cast<mutex*>(mu->mutex_ptr)->lock();
    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_mutex_release(struct ble_npl_mutex* mu) {
    static_cast<mutex*>(mu->mutex_ptr)->unlock();
    return BLE_NPL_OK;
}
}