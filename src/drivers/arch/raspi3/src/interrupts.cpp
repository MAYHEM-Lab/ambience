#include <arch/interrupts.hpp>
#include <tos/aarch64/assembly.hpp>
#include <tos/soc/bcm2837.hpp>
#include <tos/stack_storage.hpp>

extern "C" {
[[gnu::used]] void irq_handler([[maybe_unused]] uint64_t from) {
    auto controller = tos::raspi3::interrupt_controller::get(0);
    controller->irq();
}

[[gnu::used]] void fiq_handler() {
    LOG("FIQ not implemented");
}

[[gnu::used]] void serror_handler() {
    LOG("Error not implemented");
}
}

namespace tos::raspi3 {
void interrupt_controller::irq() {
    for (uint_fast32_t i = 0; i < 32; ++i) {
        if (bcm2837::INTERRUPT_CONTROLLER->irq_pending_1 == 0) {
            break;
        }
        if ((bcm2837::INTERRUPT_CONTROLLER->irq_pending_1 & (1UL << i)) == (1UL << i)) {
            bool handled = false;
            for (auto& handler : m_irq_lists.find(i)->second) {
                if (handler.function()) {
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                tos::aarch64::udf();
                // error
                tos::debug::panic("Unhandled IRQ!");
            }
        }
    }
    for (uint_fast32_t i = 0; i < 32; ++i) {
        if (bcm2837::INTERRUPT_CONTROLLER->irq_pending_2 == 0) {
            break;
        }
        if ((bcm2837::INTERRUPT_CONTROLLER->irq_pending_2 & (1UL << i)) == (1UL << i)) {
            bool handled = false;
            for (auto& handler : m_irq_lists.find(i + 32)->second) {
                if (handler.function()) {
                    handled = true;
                    break;
                }
            }
            if (!handled) {
                tos::aarch64::udf();
                // error
                tos::debug::panic("Unhandled IRQ!");
            }
        }
    }
}
} // namespace tos::raspi3