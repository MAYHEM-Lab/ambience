//
// Created by fatih on 8/16/19.
//

#include <cstdlib>
#include <stm32_hal/hal.hpp>
#include <tos/compiler.hpp>

extern "C" {
[[noreturn]] void tos_force_reset() {
    __BKPT(0);
    NVIC_SystemReset();
    TOS_UNREACHABLE();
}
}
