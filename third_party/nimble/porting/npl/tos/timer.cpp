#include <nimble/nimble_npl.h>
#include <tos/platform.hpp>

extern "C" {
ble_npl_time_t ble_npl_time_get(void) {
    return systicks.get();
}

ble_npl_error_t ble_npl_time_ms_to_ticks(uint32_t ms, ble_npl_time_t *out_ticks) {
    *out_ticks = ms;
    return BLE_NPL_OK;
}

ble_npl_error_t ble_npl_time_ticks_to_ms(ble_npl_time_t ticks, uint32_t *out_ms) {
    *out_ms = ticks;
    return BLE_NPL_OK;
}

ble_npl_time_t ble_npl_time_ms_to_ticks32(uint32_t ms) {
    return ms;
}

uint32_t ble_npl_time_ticks_to_ms32(ble_npl_time_t ticks) {
    return ticks;
}

void ble_npl_time_delay(ble_npl_time_t ticks);
}