#include <arch/generic_timer.hpp>
#include <tos/aarch64/generic_timer.hpp>
#include <tos/soc/bcm2837.hpp>

namespace tos::raspi3 {
generic_timer::generic_timer(interrupt_controller& ic, int core_num)
    : m_handler(mem_function_ref<&generic_timer::irq>(*this))
    , m_core{core_num} {
    ic.register_handler(bcm283x::irq_channels::generic_timer, m_handler);
}

void generic_timer::set_frequency(uint16_t hz) {
    m_period = aarch64::generic_timer::get_frequency() / hz;
}

void generic_timer::enable() {
    // Enable the SVC generic timer interrupt.
    bcm2837::ARM_CORE->core0_timers_irq_control = 2;
    bcm2837::INTERRUPT_CONTROLLER->enable_basic_irq = 1;
    aarch64::generic_timer::set_timeout(m_period);
    aarch64::generic_timer::enable();
}

void generic_timer::disable() {
    aarch64::generic_timer::disable();
    bcm2837::INTERRUPT_CONTROLLER->disable_basic_irq = 1;
}

bool generic_timer::irq() {
    m_fn();
    aarch64::generic_timer::set_timeout(m_period);
    return true;
}
} // namespace tos::raspi3
