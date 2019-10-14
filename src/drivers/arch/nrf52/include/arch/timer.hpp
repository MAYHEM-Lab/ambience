//
// Created by fatih on 6/8/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/timer.hpp>
#include <stdint.h>
#include <tos/function_ref.hpp>

namespace tos {
namespace nrf52 {
class timer0 : public self_pointing<timer0> {
public:
    explicit timer0(int idx);

    /**
     * Sets the function to be called on every tick of the timer.
     * @param fun function to be called on every tick.
     */
    void set_frequency(uint16_t hertz);

    /**
     * Sets the function to be called on every tick of the timer.
     * @param fun function to be called on every tick.
     */
    void set_callback(const function_ref<void()>& cb) {
        m_cb = cb;
    }

    void enable();
    void disable();

    /**
     * Gets the current value of the counter of the timer.
     * Together with get_period, these functions can be used to implement a clock.
     * @return the current value of the counter.
     */
    uint32_t get_counter() const;

    /**
     * Gets the period of the timer, i.e. the value of the counter at which the
     * tick callback is executed.
     * @return the period of the timer.
     */
    uint32_t get_period() const;

private:
    void write_ticks(uint32_t cc);

    function_ref<void()> m_cb;
    uint32_t m_ticks;
    int m_idx;
};
} // namespace nrf52

/**
 * Opens the TIMER0 peripheral on NRF52 devices
 *
 * @note TIMER0 is unavailable while the softdevice is active! Prefer using TIMER1
 * @return TIMER0 peripheral driver
 */
inline nrf52::timer0 open_impl(devs::timer_t<0>) {
    return nrf52::timer0{0};
}

inline nrf52::timer0 open_impl(devs::timer_t<1>) {
    return nrf52::timer0{1};
}
} // namespace tos
