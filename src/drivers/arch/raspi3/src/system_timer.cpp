#include <arch/system_timer.hpp>
#include <arch/detail/bcm2837.hpp>

using bcm2837::INTERRUPT_CONTROLLER;
using bcm2837::SYSTEM_TIMER;
namespace tos::raspi3 {
namespace {

}

void system_timer::irq() {
    // Clear interrupt pending for channel 0.
    m_cb();
    SYSTEM_TIMER->compare1 += 100'000;
    SYSTEM_TIMER->control_status = 2;
}

void system_timer::enable() {
    SYSTEM_TIMER->compare1 = SYSTEM_TIMER->counter_lo + 100'000;
    // Enable interrupt for channel 0.
    INTERRUPT_CONTROLLER->enable_irq_1 = 2;
}
}