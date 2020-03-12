#pragma once

#include <tos/function_ref.hpp>

namespace tos::raspi3 {
class system_timer {
public:
    system_timer() = default;

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

    void irq();

private:
    function_ref<void()> m_cb{[](void*){}};
};
}