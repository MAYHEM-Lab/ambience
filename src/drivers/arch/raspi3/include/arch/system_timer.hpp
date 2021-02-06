#pragma once

#include <arch/interrupts.hpp>
#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>

namespace tos::raspi3 {
class system_timer : public self_pointing<system_timer> {
public:
    system_timer(interrupt_controller& interrupts)
        : m_handler(function_ref<bool()>(
              [](void* ptr) -> bool {
                  auto self = static_cast<system_timer*>(ptr);
                  return self->irq();
              },
              this)) {
        interrupts.register_handler(bcm283x::irq_channels::system_timer, m_handler);
    }

    /**
     * Sets the function to be called on every tick of the timer.
     * @param fun function to be called on every tick.
     */
    void set_frequency(uint16_t hertz) {
        m_freq = hertz;
    }

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

    bool irq();

private:
    uint16_t m_freq;
    irq_handler m_handler;
    function_ref<void()> m_cb{[](void*) {}};
};
} // namespace tos::raspi3