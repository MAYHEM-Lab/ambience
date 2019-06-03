//
// Created by fatih on 6/2/19.
//

#pragma once

#include <memory>
#include <common/ble_srv_common.h>
#include <tos/print.hpp>
#include <tos/ring_buf.hpp>
#include <tos/semaphore.hpp>
#include <tos/fixed_fifo.hpp>

#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#include <arch/ble.hpp>

namespace bakir
{
class ble_service
{
public:
    static constexpr ble_uuid128_t uuid{
        {0x08,0x9E,0x33,0x3F,0x66,0xE8,0x4D,0x30,0xA4,0x23,0x96,0xF1,0x36,0xA2,0x5C,0x02}
    };

    ble_service(const ble_service&) = delete;
    ble_service(ble_service&&) = delete;

    ble_service() {
        m_obs.obs.handler = [](ble_evt_t const* ev, void* ctx) {
            auto ptr = static_cast<ble_service*>(ctx);
            ptr->handle(*ev);
        };
        m_obs.obs.p_context = this;
        tos::nrf52::nrf_events.attach(m_obs);
    }

    tos::span<char> read(tos::span<char> b) {
        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        tos::kern::busy();
        while (total < len) {
            rx_len.down();
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        tos::kern::unbusy();
        return b.slice(0, total);
    }

    ~ble_service()
    {
        tos::nrf52::nrf_events.remove(m_obs);
    }

private:
    template <class LogT>
    friend std::unique_ptr<ble_service> make_ble_service(LogT&& log);

    void handle(ble_evt_t const& ev) {
        switch (ev.header.evt_id)
        {
            case BLE_GAP_EVT_CONNECTED:
                //on_connect(p_nus, p_ble_evt);
                break;

            case BLE_GATTS_EVT_WRITE:
            {
                ble_gatts_evt_write_t const * p_evt_write = &ev.evt.gatts_evt.params.write;
                if ((p_evt_write->handle == m_rx_handles.value_handle))
                {
                    tos::span<const uint8_t> data{p_evt_write->data, p_evt_write->len};
                    while(!data.empty() && rx_buf.size() != rx_buf.capacity())
                    {
                        rx_buf.push(data[0]);
                        data = data.slice(1);
                        rx_len.up_isr();
                    }
                }
            }
                break;
            case BLE_GATTS_EVT_HVN_TX_COMPLETE:
                //on_hvx_tx_complete(p_nus, p_ble_evt);
                break;

            default:
                // No implementation needed.
                break;
        }
    }

    uint16_t m_service_handle;
    ble_gatts_char_handles_t m_rx_handles;
    ble_uuid_t m_uuid;
    tos::nrf52::ble_observer m_obs;

    tos::fixed_fifo<uint8_t, 64, tos::ring_buf> rx_buf;
    tos::semaphore rx_len{0};
};

template <class LogT>
inline std::unique_ptr<ble_service> make_ble_service(LogT&& log)
{
    auto res = std::make_unique<ble_service>();
    ble_uuid128_t nus_base_uuid = ble_service::uuid;
    uint8_t uuid_type;
    auto err_code = sd_ble_uuid_vs_add(&nus_base_uuid, &uuid_type);
    tos::println(log, int(err_code), int(uuid_type));

    res->m_uuid.type = uuid_type;
    res->m_uuid.uuid = 1;

    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &res->m_uuid,
                                        &res->m_service_handle);
    tos::println(log, int(err_code), int(res->m_service_handle));

    ble_add_char_params_t add_char_params{};

    add_char_params.uuid                     = 10;
    add_char_params.uuid_type                = uuid_type;
    add_char_params.max_len                  = 20;
    add_char_params.init_len                 = 1;
    add_char_params.is_var_len               = true;
    add_char_params.char_props.write_wo_resp = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(res->m_service_handle, &add_char_params, &res->m_rx_handles);
    tos::println(log, int(err_code), int(res->m_rx_handles.value_handle));

    return res;
}
}