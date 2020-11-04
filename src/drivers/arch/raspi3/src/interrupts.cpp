#include <arch/detail/bcm2837.hpp>
#include <arch/interrupts.hpp>
#include <tos/aarch64/interrupts.hpp>

extern "C" {
[[gnu::used]] void exc_handler(tos::aarch64::exception_type type,
                               uint64_t esr,
                               uint64_t elr,
                               uint64_t spsr,
                               uint64_t far) {
    auto controller = tos::raspi3::interrupt_controller::get(0);

    switch (type) {
    case tos::aarch64::exception_type::synchronous:
        controller->synchronous(esr, elr, spsr, far);
        break;
    case tos::aarch64::exception_type::irq:
        controller->irq(esr, elr, spsr, far);
        break;
    case tos::aarch64::exception_type::fiq:
        controller->fiq(esr, elr, spsr, far);
        break;
    case tos::aarch64::exception_type::system_error:
        controller->serror(esr, elr, spsr, far);
        break;
    }
}
}

namespace tos::raspi3 {
void interrupt_controller::synchronous(uint64_t esr,
                                       uint64_t elr,
                                       uint64_t spsr,
                                       uint64_t far) {
    LOG(reinterpret_cast<void*>(esr),
        reinterpret_cast<void*>(elr),
        reinterpret_cast<void*>(spsr),
        far);
//    tos::debug::panic("Synchronous interrupts not implemented");
}

void interrupt_controller::irq(uint64_t esr,
                               [[maybe_unused]] uint64_t elr,
                               [[maybe_unused]] uint64_t spsr,
                               [[maybe_unused]] uint64_t far) {
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
                // error
                tos::debug::panic("Unhandled IRQ!");
            }
        }
    }
}

void interrupt_controller::fiq(uint64_t esr, uint64_t elr, uint64_t spsr, uint64_t far) {
    tos::debug::panic("FIQ interrupts not implemented");
}

void interrupt_controller::serror(uint64_t esr,
                                  uint64_t elr,
                                  uint64_t spsr,
                                  uint64_t far) {
    tos::debug::panic("SError interrupts not implemented");
}
} // namespace tos::raspi3