#pragma once

#include <arch/interrupts.hpp>
#include <cstdint>
#include <tos/self_pointing.hpp>

namespace tos::raspi3 {
class generic_timer : public self_pointing<generic_timer> {
public:
    generic_timer(interrupt_controller& ic, int core_num);

    void enable();
    void disable();

    void set_frequency(uint16_t hz);
    void set_callback(function_ref<void()> fn) {
        m_fn = fn;
    }

private:
    bool irq();

    irq_handler m_handler;
    int m_core;
    uint64_t m_period;
    function_ref<void()> m_fn{[](auto...) {}};
};
} // namespace tos::raspi3
