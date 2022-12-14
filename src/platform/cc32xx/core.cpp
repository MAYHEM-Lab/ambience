//
// Created by fatih on 10/31/19.
//

#include <cstdlib>
#include <tos/compiler.hpp>
#include <cstdint>
#include <tos/platform.hpp>

extern "C" {
#include <NoRTOS.h>
#include <ti/drivers/dpl/HwiP.h>
}

namespace tos::platform {
static uintptr_t interrupt_disable_key;

bool interrupts_disabled() {
    // TODO: This needs to be implemented
    return false;
}

void enable_interrupts() {
    HwiP_restore(interrupt_disable_key);
}

void disable_interrupts() {
    interrupt_disable_key = HwiP_disable();
}

[[noreturn]]
void force_reset() {
    while (true) {
        asm volatile("BKPT 0");
    }
}
}
