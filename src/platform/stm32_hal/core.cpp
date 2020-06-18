//
// Created by fatih on 8/16/19.
//

#include <cstdlib>
#include <stm32_hal/hal.hpp>
#include <tos/compiler.hpp>
#include <tos/platform.hpp>

namespace tos::platform {
void force_reset() {
    __BKPT(0);
    NVIC_SystemReset();
    TOS_UNREACHABLE();
}
} // namespace tos::platform