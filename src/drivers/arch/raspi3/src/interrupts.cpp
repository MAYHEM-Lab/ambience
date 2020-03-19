#include <arch/interrupts.hpp>
#include <arch/detail/bcm2837.hpp>

extern "C" {
[[gnu::used]] void
exc_handler(uint64_t type, uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) {
    if (type == 1) {
        auto controller = tos::raspi3::interrupt_controller::get(0);
        controller->irq(esr, elr, spsr, far);
    }
}
}

namespace tos::raspi3 {
void interrupt_controller::irq(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) {
    for (int i = 0; i < 32; ++i) {
        if (bcm2837::INTERRUPT_CONTROLLER->irq_pending_1 == 0) {
            break;
        }
        if ((bcm2837::INTERRUPT_CONTROLLER->irq_pending_1 & (1 << i)) == (1 << i)) {
            bool handled = false;
            for (auto& handler : m_irq_lists.find(i)->second) {
                if (handler.function()) {
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                // error
                tos::debug::panic("Unhandled IRQ!");
            }
        }
    }
}
} // namespace tos::raspi3