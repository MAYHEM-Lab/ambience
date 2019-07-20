//
// Created by fatih on 6/2/19.
//

#include <arch/ble/advertising.hpp>
#include <app_error.h>

using namespace tos::nrf52;

static ble_uuid_t m_adv_uuids[]          =
{
    {0xDDDD, BLE_UUID_TYPE_VENDOR_BEGIN}
};

namespace tos
{
namespace nrf52
{
ble_advertising_t m_advertising;

advertising::advertising(std::chrono::milliseconds duration, std::chrono::milliseconds interval)
    : tracked_driver(0) {
    ble_advertising_init_t init{};

    init.advdata.name_type          = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags              = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = std::size(m_adv_uuids);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = interval.count() / 0.625; /**< The advertising interval (in units of 0.625 ms. */
    init.config.ble_adv_fast_timeout  = duration.count() / 10; /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
    init.evt_handler = &advertising::evt_handler;

    auto err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    m_ble_obs.obs.handler = ble_advertising_on_ble_evt;
    m_ble_obs.obs.p_context = &m_advertising;

    m_soc_obs.obs.handler = ble_advertising_on_sys_evt;
    m_soc_obs.obs.p_context = &m_advertising;

    nrf_events.attach(m_ble_obs);
    nrf_events.attach(m_soc_obs);

    // https://devzone.nordicsemi.com/f/nordic-q-a/33504/what-does-app_ble_conn_cfg_tag-do
    constexpr auto tag = 1;

    ble_advertising_conn_cfg_tag_set(&m_advertising, tag);
}
}
}