//
// Created by fatih on 6/2/19.
//

#include <common/driver_base.hpp>
#include <ble_advertising/ble_advertising.h>
#include <arch/ble/events.hpp>
#include <tos/expected.hpp>
#include <tos/semaphore.hpp>

namespace tos
{
namespace nrf52 {
extern ble_advertising_t m_advertising;

enum class advertising_errors
{
    module_not_initialized = NRF_ERROR_INVALID_STATE
};

/**
 * This type serves as a thin wrapper around the nordic ble_advertising module
 *
 * Currently, we only support the fast advertisement mode
 */
class advertising : public tracked_driver<advertising, 1>, non_copy_movable
{
public:
    advertising(std::chrono::milliseconds duration, std::chrono::milliseconds interval);

    expected<void, advertising_errors>
    start() {
        m_fin.down();
        uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        if (err_code == NRF_SUCCESS) return {};
        return unexpected((advertising_errors)err_code);
    }

    /**
     * Attaches an event handler to be called when the advertisement
     * finishes (due to a timeout right now).
     *
     * @note The function is called from an ISR context!
     * @param fun function to be called
     */
    void on_finish(tos::function_ref<void()> fun,
            const tos::no_interrupts& = tos::int_guard{})
    {
        m_on_finish = fun;
    }

    ~advertising()
    {
        m_fin.down();
        nrf_events.remove(m_ble_obs);
        nrf_events.remove(m_soc_obs);
    }

private:
    void handler(ble_adv_evt_t ble_adv_evt) {
        switch (ble_adv_evt)
        {
            case BLE_ADV_EVT_FAST:
                // advertisement started, should we do anything here?
                break;
            case BLE_ADV_EVT_IDLE:
                // advertisement finished
                m_fin.up_isr();
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

    tos::semaphore m_fin{1};
    tos::function_ref<void()> m_on_finish{[](void*){}};
    ble_observer m_ble_obs;
    soc_observer m_soc_obs;
};
}
}