//
// Created by fatih on 10/31/19.
//

#include <cstdlib>
#include <tos/compiler.hpp>
#include <cstdint>

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

static uintptr_t interrupt_disable_key;
void tos_enable_interrupts() {
    HwiP_restore(interrupt_disable_key);
}

void tos_disable_interrupts() {
    interrupt_disable_key = HwiP_disable();
}

void* tos_stack_alloc(size_t sz) { return malloc(sz); }

void tos_stack_free(void* ptr) { return free(ptr); }
}
