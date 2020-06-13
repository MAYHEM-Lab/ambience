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

extern "C" {
[[noreturn]] void tos_force_reset() {
    while (true) {
        asm volatile("BKPT 0");
    }
}
}

namespace tos::platform {
static uintptr_t interrupt_disable_key;

void enable_interrupts() {
    HwiP_restore(interrupt_disable_key);
}

void disable_interrupts() {
    interrupt_disable_key = HwiP_disable();
}
}