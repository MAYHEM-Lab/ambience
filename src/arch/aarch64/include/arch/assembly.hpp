#pragma once

namespace tos::aarch64 {
void nop() {
    asm volatile("nop");
}
void breakpoint() {
    asm volatile("brk #0");
}
} // namespace tos::aarch64