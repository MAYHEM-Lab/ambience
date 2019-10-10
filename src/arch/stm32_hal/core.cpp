//
// Created by fatih on 8/16/19.
//

#include <cstdlib>
#include <stm32_hal/hal.hpp>
#include <tos/compiler.hpp>

extern "C" {
void NORETURN tos_force_reset() {
    __BKPT(0);
    NVIC_SystemReset();
    TOS_UNREACHABLE();
}

void* tos_stack_alloc(size_t sz) { return malloc(sz); }

void tos_stack_free(void* ptr) { return free(ptr); }
}
