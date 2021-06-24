#pragma once

namespace tos::platform {
inline void enable_interrupts() {}
inline void disable_interrupts() {}
inline bool interrupts_disabled() {
    return true;
}

[[noreturn]]
inline void force_reset() {
    TOS_UNREACHABLE();
}
}
