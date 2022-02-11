#pragma once

#include <tos/core/arch_fwd.hpp>
#include <tos/utility.hpp>

namespace tos::platform {
inline void enable_interrupts() {
}
inline void disable_interrupts() {
}
inline bool interrupts_disabled() {
    return true;
}

[[noreturn]] inline void force_reset() {
    tos::unreachable();
}

cur_arch::address_space& get_kernel_address_space();
} // namespace tos::platform
