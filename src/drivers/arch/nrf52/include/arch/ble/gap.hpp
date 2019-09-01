//
// Created by fatih on 8/31/19.
//

#pragma once

#include "common.hpp"
#include "events.hpp"
#include "gatt.hpp"

#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/interrupt.hpp>

namespace tos {
namespace nrf52 {
struct connection_event {};

/**
 * This type manages the GAP services provided by the nRF52 Soft Device
 * driver.
 */
class gap : public tos::non_copy_movable {
public:
    gap(gatt&, std::string_view name);

    void on_connection(tos::function_ref<void(connection_event&&)> fun,
                       const tos::no_interrupts& = tos::int_guard{}) {
        m_on_conn = fun;
    }

    expected<void, ble_errors> set_device_name(std::string_view name);

    void operator()(const ble_evt_t& ev) {
        if (ev.header.evt_id == BLE_GAP_EVT_CONNECTED) {
            m_on_conn(connection_event{});
        }
    }

private:
    ble_observer m_observer;

    tos::function_ref<void(connection_event&&)> m_on_conn{
        [](connection_event&&, void*) {}};
};
} // namespace nrf52
} // namespace tos