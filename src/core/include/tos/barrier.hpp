//
// Created by fatih on 10/5/18.
//

#pragma once

namespace tos {
namespace detail {
inline void memory_barrier() {
    asm volatile("" ::: "memory");
}
} // namespace detail
} // namespace tos