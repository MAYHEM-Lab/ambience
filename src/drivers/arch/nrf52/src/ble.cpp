//
// Created by fatih on 6/2/19.
//

#include <arch/ble.hpp>
#include <nrf_ble_gatt/nrf_ble_gatt.h>

using namespace tos::nrf52;

static nrf_sdh_soc_evt_observer_t tos_soc_observer
    __attribute__((section(".sdh_soc_observers1")))
    __attribute__((used)){[](uint32_t ev, void* ctx) {
                              auto events = (nrf_events_t*)ctx;
                              events->on_soc_evt(ev);
                          },
                          &nrf_events};

static nrf_sdh_ble_evt_observer_t tos_ble_observer
    __attribute__((section(".sdh_ble_observers1")))
    __attribute__((used)){[](ble_evt_t const* ev, void* ctx) {
                              auto events = (nrf_events_t*)ctx;
                              events->on_ble_evt(ev);
                          },
                          &nrf_events};

namespace tos::nrf52 {
nrf_events_t nrf_events;
}

static void ble_evt_handler(ble_evt_t const* p_ble_evt, void*) {
    uint32_t err_code;

    switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        ble_gap_phys_t const phys = {
            BLE_GAP_PHY_AUTO,
            BLE_GAP_PHY_AUTO,
        };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
    } break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // Pairing not supported

        err_code = sd_ble_gap_sec_params_reply(p_ble_evt->evt.gap_evt.conn_handle,
                                               BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                               nullptr,
                                               nullptr);
        APP_ERROR_CHECK(err_code);
        break;

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        // No system attributes have been stored.
        err_code =
            sd_ble_gatts_sys_attr_set(p_ble_evt->evt.gap_evt.conn_handle, nullptr, 0, 0);
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

static nrf_sdh_ble_evt_observer_t m_ble_observer
    __attribute__((section(".sdh_ble_observers3")))
    __attribute__((used)) = {ble_evt_handler, nullptr};
