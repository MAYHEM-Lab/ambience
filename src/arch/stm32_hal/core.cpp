//
// Created by fatih on 8/16/19.
//

#include <cstdlib>
#include <stm32f7xx_hal.h>
#include <tos/compiler.hpp>

extern "C" {
void NORETURN tos_force_reset() { NVIC_SystemReset(); }

void* tos_stack_alloc(size_t sz) { return malloc(sz); }

void tos_stack_free(void* ptr) { return free(ptr); }
}
