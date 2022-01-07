#include "tos/debug/log.hpp"
#include <nimble/nimble_npl.h>
#include <nrfx.h>

namespace {
void (*radio_irq_handler)();
void (*rng_irq_handler)();
} // namespace

extern "C" void RADIO_IRQHandler() {
    radio_irq_handler();
}

extern "C" void RNG_IRQHandler() {
    rng_irq_handler();
}

extern "C" {
void ble_npl_hw_set_isr(int irqn, void (*addr)(void)) {
    switch (irqn) {
    case RADIO_IRQn:
        LOG("Setting RADIO IRQ");
        radio_irq_handler = addr;
        return;
    case RNG_IRQn:
        LOG("Setting RNG IRQ");
        rng_irq_handler = addr;
        return;
    }

    LOG_ERROR("Unknown IRQ:", irqn);
}
}