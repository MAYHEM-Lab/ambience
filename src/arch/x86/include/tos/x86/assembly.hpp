#pragma once

namespace tos::x86 {
inline void breakpoint() {
    asm volatile("int3");
}

inline void nop() {
    asm volatile("nop");
}
} // namespace tos::x86
