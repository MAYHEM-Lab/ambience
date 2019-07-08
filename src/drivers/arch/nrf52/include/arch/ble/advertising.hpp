//
// Created by fatih on 6/2/19.
//

#include <common/driver_base.hpp>
#include <ble_advertising/ble_advertising.h>
#include <arch/ble/events.hpp>
#include <tos/expected.hpp>

namespace tos
{
namespace nrf52 {
extern ble_advertising_t m_advertising;

enum class advertising_errors
{
    module_not_initialized = NRF_ERROR_INVALID_STATE
};

class advertising : public tracked_driver<advertising, 1>
{
public:
    advertising();

    advertising(const advertising&) = delete;
    advertising(advertising&&) = delete;

    expected<void,advertising_errors>
    start() {
        uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        if (err_code == NRF_SUCCESS) return {};
        return unexpected((advertising_errors)err_code);
    }

    void on_finish(tos::function_ref<void()> fun)
    {
        m_on_finish = std::move(fun);
    }

    ~advertising()
    {
        nrf_events.remove(m_ble_obs);
        nrf_events.remove(m_soc_obs);
    }

private:

    void handler(ble_adv_evt_t ble_adv_evt) {
        switch (ble_adv_evt)
        {
            case BLE_ADV_EVT_FAST:
                //err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
                //APP_ERROR_CHECK(err_code);
                break;
            case BLE_ADV_EVT_IDLE:
                // advertisement finished
                m_on_finish();
                break;
            default:
                break;
        }
    }

    static void evt_handler(ble_adv_evt_t ble_adv_evt)
    {
        auto inst = advertising::get(0);
        if (!inst){
            //TODO: error
            return;
        }
        inst->handler(ble_adv_evt);
    }

    tos::function_ref<void()> m_on_finish{[](void*){}};
    ble_observer m_ble_obs;
    soc_observer m_soc_obs;
};
}
}