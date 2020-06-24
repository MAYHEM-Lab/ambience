#pragma once

#include <tos/compiler.hpp>

namespace tos::xtensa {
ALWAYS_INLINE
void nop() {
    asm volatile("nop");
}
}