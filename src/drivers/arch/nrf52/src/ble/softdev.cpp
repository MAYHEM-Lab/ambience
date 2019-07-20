//
// Created by fatih on 6/2/19.
//

#include <arch/ble/softdev.hpp>
#include <arch/ble/advertising.hpp>

namespace tos {
namespace nrf52 {
expected<void, softdev_errors> softdev::set_tx_power(int8_t power) {
    auto err = sd_ble_gap_tx_power_set(BLE_GAP_TX_POWER_ROLE_ADV, m_advertising.adv_handle, power);
    if (err == NRF_SUCCESS) return {};
    return unexpected(softdev_errors(err));
}

softdev::softdev() {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    //APP_ERROR_CHECK(err_code);
    if (err_code != NRF_SUCCESS)
    {
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

expected<void, softdev_errors> softdev::set_device_name(std::string_view name) {
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    auto err_code = sd_ble_gap_device_name_set(&sec_mode,
                                               (const uint8_t *) name.data(),
                                               name.size());

    if (err_code != NRF_SUCCESS)
    {
        return unexpected(softdev_errors(err_code));
    }

    return {};
}
}
}