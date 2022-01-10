#include "tos/semaphore.hpp"

#include "tos/interrupt.hpp"
#include "tos/thread.hpp"
#include <chrono>
#include <cstdint>
#include <limits>
#include <nimble/nimble_npl.h>
#include <tos/mutex.hpp>
#include "common.hpp"

using namespace tos;

extern "C" {
ble_npl_error_t ble_npl_sem_init(struct ble_npl_sem* sem, uint16_t tokens) {
    if (tokens > std::numeric_limits<int16_t>::max()) {
        return BLE_NPL_ERROR;
    }

    sem->sem_ptr = new semaphore{static_cast<int16_t>(tokens)};

    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_sem_pend(struct ble_npl_sem* sem, ble_npl_time_t timeout) {
    if (timeout == BLE_NPL_TIME_FOREVER) {
        static_cast<semaphore*>(sem->sem_ptr)->down();
        return BLE_NPL_OK;
    }

    if (timeout == 0) {
        tos::int_guard ig;
        return try_down_isr(*static_cast<semaphore*>(sem->sem_ptr)) ? BLE_NPL_OK : BLE_NPL_TIMEOUT;
    }

    // tos::this_thread::yield();
    // LOG("Timeout down", timeout);
    auto down_res = static_cast<semaphore*>(sem->sem_ptr)
                        ->down(*tos::nimble::alarm, std::chrono::milliseconds(timeout));
    if (down_res == tos::sem_ret::normal) {
        return BLE_NPL_OK;
    }
    return BLE_NPL_TIMEOUT;
}

ble_npl_error_t ble_npl_sem_release(struct ble_npl_sem* sem) {
    static_cast<semaphore*>(sem->sem_ptr)->up();

    return BLE_NPL_OK;
}

uint16_t ble_npl_sem_get_count(struct ble_npl_sem* sem) {
    return std::max<int16_t>(0, get_count(*static_cast<semaphore*>(sem->sem_ptr)));
}
}