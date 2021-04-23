#include <arch/interrupts.hpp>
#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/semihosting.hpp>
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
            do_irq(i);
        }
    }

    for (uint_fast32_t i = 0; i < 32; ++i) {
        if (bcm2837::INTERRUPT_CONTROLLER->irq_pending_2 == 0) {
            break;
        }
        if ((bcm2837::INTERRUPT_CONTROLLER->irq_pending_2 & (1UL << i)) == (1UL << i)) {
            do_irq(i + 32);
        }
    }

    if (bcm2837::ARM_CORE->core0_irq_source & 0b10) {
        do_irq(static_cast<int>(bcm283x::irq_channels::generic_timer));
    }

    m_post_irq();
}

void interrupt_controller::do_irq(int channel) {
    if (!try_irq(channel)) {
        aarch64::semihosting::write0("Unhandled IRQ ");
        aarch64::semihosting::write0(itoa(channel).data());
        aarch64::semihosting::write0("!\n");
        while (true)
            ;
    }
}

bool interrupt_controller::try_irq(int channel) {
    auto iter = m_irq_lists.find(channel);
    if (iter == m_irq_lists.end()) {
        return false;
    }
    bool handled = false;

    for (auto& handler : iter->second) {
        if (handler.function()) {
            handled = true;
            break;
        }
    }
    return handled;
}

void interrupt_controller::set_post_irq(function_ref<void()> handler) {
    m_post_irq = handler;
}

void interrupt_controller::reset_post_irq() {
    m_post_irq = function_ref<void()>([](void*) {});
}
} // namespace tos::raspi3