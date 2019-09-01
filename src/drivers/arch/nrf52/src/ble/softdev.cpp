//
// Created by fatih on 6/2/19.
//

#include <arch/ble/softdev.hpp>

namespace tos {
namespace nrf52 {
softdev::softdev() {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    // APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS) {
        tos::this_thread::block_forever();
    }

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(1, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    err_code = sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
    APP_ERROR_CHECK(err_code);
}
} // namespace nrf52
} // namespace tos