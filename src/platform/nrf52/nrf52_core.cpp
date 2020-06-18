//
// Created by fatih on 2/22/19.
//

#include <cstdlib>
#include <nrf.h>
#include <tos/arch.hpp>
#include <tos/compiler.hpp>

namespace tos::platform {
[[noreturn]] void force_reset() {
    __BKPT(0);
    NVIC_SystemReset();
    TOS_UNREACHABLE();
}
} // namespace tos::platform
