//
// Created by fatih on 10/5/18.
//

#pragma once

#include <tos/compiler.hpp>

namespace tos {
namespace detail {
ALWAYS_INLINE
void memory_barrier() {
    asm volatile("" ::: "memory");
}
} // namespace detail
} // namespace tos