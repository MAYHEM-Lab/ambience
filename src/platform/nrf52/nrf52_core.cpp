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
}
