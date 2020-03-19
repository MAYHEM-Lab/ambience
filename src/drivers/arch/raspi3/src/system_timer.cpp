#include <arch/detail/bcm2837.hpp>
#include <arch/system_timer.hpp>


using bcm2837::INTERRUPT_CONTROLLER;
using bcm2837::SYSTEM_TIMER;
namespace tos::raspi3 {
namespace {}

uint32_t system_timer::get_counter() const {
    return SYSTEM_TIMER->counter_lo;
}

bool system_timer::irq() {
    // Clear interrupt pending for channel 0.
    m_cb();
    SYSTEM_TIMER->compare1 = SYSTEM_TIMER->counter_lo + 1'000'000 / m_freq;
    SYSTEM_TIMER->control_status = 2;

    bcm2837::SYSTEM_TIMER->control_status = 2;
    return true;
}

void system_timer::enable() {
    SYSTEM_TIMER->compare1 = SYSTEM_TIMER->counter_lo + 1'000'000 / m_freq;
    // Enable interrupt for channel 0.
    INTERRUPT_CONTROLLER->enable_irq_1 = 2;
}
void system_timer::disable() {
    bcm2837::INTERRUPT_CONTROLLER->disable_irq_1 = 2;
}
} // namespace tos::raspi3