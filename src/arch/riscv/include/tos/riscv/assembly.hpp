#pragma once

#include <tos/compiler.hpp>

namespace tos::riscv {
inline void breakpoint() {
    asm volatile("ebreak");
}
} // namespace tos::riscv
