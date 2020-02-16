//
// Created by fatih on 2/22/19.
//

#include <cstdlib>
#include <tos/compiler.hpp>
#include <tos_arch.hpp>

extern "C" {
[[noreturn]] void tos_force_reset() {
    __BKPT(0);
    NVIC_SystemReset();
    TOS_UNREACHABLE();
}

void* tos_stack_alloc(size_t sz) {
    return malloc(sz);
}

void tos_stack_free(void* ptr) {
    return free(ptr);
}
}
