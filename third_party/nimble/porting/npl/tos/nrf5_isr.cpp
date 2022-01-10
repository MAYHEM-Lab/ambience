#include "tos/compiler.hpp"
#include "tos/debug/log.hpp"
#include <nimble/nimble_npl.h>
#include <nrfx.h>

namespace {
TOS_NO_OPTIMIZE
void noop() {
    while (true) {
        asm volatile("nop");
    }
}
TOS_NO_OPTIMIZE
void noop1() {
    while (true) {
        asm volatile("nop");
        asm volatile("nop");
    }
}

TOS_NO_OPTIMIZE
void noop2() {
    while (true) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
}

TOS_NO_OPTIMIZE
void noop3() {
    while (true) {
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
        asm volatile("nop");
    }
}
void (*radio_irq_handler)() = &noop;
void (*rng_irq_handler)() = &noop1;
void (*timer4_irq_handler)() = &noop2;
void (*rtc0_irq_handler)() = &noop3;
} // namespace

extern "C" void RADIO_IRQHandler() {
    radio_irq_handler();
}

extern "C" void RNG_IRQHandler() {
    rng_irq_handler();
}

extern "C" void TIMER4_IRQHandler() {
    timer4_irq_handler();
}

extern "C" void RTC0_IRQHandler() {
    rtc0_irq_handler();
}

extern "C" {
void ble_npl_hw_set_isr(int irqn, void (*addr)(void)) {
    switch (irqn) {
    case RADIO_IRQn:
        // LOG("Setting RADIO IRQ");
        radio_irq_handler = addr;
        return;
    case RNG_IRQn:
        // LOG("Setting RNG IRQ");
        rng_irq_handler = addr;
        return;
    case TIMER4_IRQn:
        // LOG("Setting TIMER4 IRQ");
        timer4_irq_handler = addr;
        return;
    case RTC0_IRQn:
        // LOG("Setting TIMER4 IRQ");
        rtc0_irq_handler = addr;
        return;
    }

    LOG_ERROR("Unknown IRQ:", irqn);
}
void nrf52_clock_hfxo_request(void) {
    // NRF_CLOCK->TASKS_HFCLKSTART = 1;
}

void nrf52_clock_hfxo_release(void) {
    // NRF_CLOCK->TASKS_HFCLKSTOP = 1;
}
}