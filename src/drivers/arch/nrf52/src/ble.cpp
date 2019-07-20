//
// Created by fatih on 6/2/19.
//

#include <arch/ble.hpp>
#include <nrf_ble_gatt/nrf_ble_gatt.h>

using namespace tos::nrf52;

static nrf_sdh_soc_evt_observer_t tos_soc_observer
    __attribute__((section(".sdh_soc_observers1"))) __attribute__((used))
    { [](uint32_t ev, void* ctx) {
        auto events = (nrf_events_t*) ctx;
        events->on_soc_evt(ev);
    }, &nrf_events };

static nrf_sdh_ble_evt_observer_t tos_ble_observer
    __attribute__((section(".sdh_ble_observers1"))) __attribute__((used))
    { [](ble_evt_t const* ev, void* ctx) {
        auto events = (nrf_events_t*) ctx;
        events->on_ble_evt(ev);
    }, &nrf_events };

namespace tos::nrf52{
nrf_events_t nrf_events;
}

static nrf_ble_gatt_t m_gatt;
// Calls the event handler of the GATT library
static nrf_sdh_ble_evt_observer_t m_gatt_obs
__attribute__((section(".sdh_ble_observers1")))
__attribute__((used)) = {
    nrf_ble_gatt_on_ble_evt,
    &m_gatt
};

static void ble_evt_handler(ble_evt_t const * p_ble_evt, void *)
{
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            ble_gap_phys_t const phys =
                {
                    BLE_GAP_PHY_AUTO,
                    BLE_GAP_PHY_AUTO,
                };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
            // Pairing not supported

            err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle, BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP, nullptr, nullptr);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_SYS_ATTR_MISSING:
            // No system attributes have been stored.
            err_code = sd_ble_gatts_sys_attr_set(p_ble_evt->evt.gap_evt.conn_handle, nullptr, 0, 0);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            break;
    }
}

static nrf_sdh_ble_evt_observer_t m_ble_observer __attribute__((section(".sdh_ble_observers3")))__attribute__((used)) = {
    ble_evt_handler, nullptr};

static void gatt_evt_handler(nrf_ble_gatt_t *, nrf_ble_gatt_evt_t const *){}

void gatt_init()
{
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    APP_ERROR_CHECK(err_code);
}

void gap_params_init()
{
    ble_gap_conn_params_t gap_conn_params {};

    gap_conn_params.min_conn_interval = MSEC_TO_UNITS(40, UNIT_1_25_MS);
    gap_conn_params.max_conn_interval = MSEC_TO_UNITS(150, UNIT_1_25_MS);
    gap_conn_params.slave_latency     = 0;
    gap_conn_params.conn_sup_timeout  = MSEC_TO_UNITS(4000, UNIT_10_MS);

    auto err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}
