#pragma once

namespace tos::aarch64::intrin {
void nop() {
    asm volatile("nop");
}
void bkpt() {
    asm volatile("brk #0");
}
} // namespace tos::aarch64::intrin