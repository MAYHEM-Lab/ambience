#pragma once

namespace tos::x86 {
inline void breakpoint() {
    asm volatile("int3");
}
} // namespace tos::x86
