//
// Created by fatih on 8/31/19.
//

#include <arch/ble/gap.hpp>

namespace {
void gap_params_init() {
    ble_gap_conn_params_t gap_conn_params{};

    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(40, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(150, UNIT_1_25_MS);
    gap_conn_params.slave_latency = 0;
    gap_conn_params.conn_sup_timeout = MSEC_TO_UNITS(4000, UNIT_10_MS);

    auto err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    if (err_code != NRF_SUCCESS) {
        // error
    }
}
} // namespace

namespace tos {
namespace nrf52 {
gap::gap(gatt&, std::string_view name)
    : m_observer{*this} {
    nrf_events.attach(m_observer);
    gap_params_init();
    set_device_name(name);
}

expected<void, ble_errors> gap::set_device_name(std::string_view name) {
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    auto err_code =
        sd_ble_gap_device_name_set(&sec_mode, (const uint8_t*)name.data(), name.size());

    if (err_code != NRF_SUCCESS) {
        return unexpected(ble_errors(err_code));
    }

    return {};
}
} // namespace nrf52
} // namespace tos