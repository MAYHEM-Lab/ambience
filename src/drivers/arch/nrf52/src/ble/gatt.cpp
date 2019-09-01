//
// Created by fatih on 8/31/19.
//

#include <arch/ble/gatt.hpp>
#include <nrf_ble_gatt/nrf_ble_gatt.h>

namespace tos {
namespace nrf52 {

namespace {
nrf_ble_gatt_t m_gatt;
// Calls the event handler of the GATT library
nrf_sdh_ble_evt_observer_t m_gatt_obs __attribute__((section(".sdh_ble_observers1")))
__attribute__((used)) = {nrf_ble_gatt_on_ble_evt, &m_gatt};

void gatt_evt_handler(nrf_ble_gatt_t*, nrf_ble_gatt_evt_t const*) {
}
} // namespace

gatt::gatt() {
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    if (err_code != NRF_SUCCESS) {
        // error
    }

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    if (err_code != NRF_SUCCESS) {
        // error
    }
}
} // namespace nrf52
} // namespace tos