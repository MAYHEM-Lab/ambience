//
// Created by Mehmet Fatih BAKIR on 28/04/2018.
//

#pragma once

#include <cstdint>
#include <tos/arch.hpp>
#include <tos/barrier.hpp>
#include <tos/debug/assert.hpp>
#include <tos/platform.hpp>

namespace tos {
namespace global {
extern int8_t disable_depth;
extern bool should_enable;
} // namespace global
namespace kern {
inline void do_disable_interrupts([[maybe_unused]] void* ret_addr) {
    tos::detail::memory_barrier();
    Assert(global::disable_depth >= 0);
    if (global::disable_depth == 0) {
        if (platform::interrupts_disabled()) {
            global::should_enable = false;
        } else {
            platform::disable_interrupts();
            global::should_enable = true;
        }
    }
    global::disable_depth++;
    tos::detail::memory_barrier();
}

/**
 * Decrements the interrupt disable count and if
 * it reaches zero, globally enables interrupts.
 *
 * Must be matched by a previous `disable_interrupts` call.
 */
inline void do_enable_interrupts([[maybe_unused]] void *ret_addr) {
    tos::detail::memory_barrier();
    Assert(global::disable_depth > 0);
    global::disable_depth--;
    if (global::disable_depth == 0) {
        if (global::should_enable) {
            global::should_enable = false;
            platform::enable_interrupts();
        }
    }
    tos::detail::memory_barrier();
}
NO_INLINE
inline void disable_interrupts() {
    do_disable_interrupts(__builtin_return_address(0));
}
NO_INLINE
inline void enable_interrupts() {
    do_enable_interrupts(__builtin_return_address(0));
}

inline void disable_interrupts(void* ptr) {
    do_disable_interrupts(ptr);
}

inline void enable_interrupts(void* ptr) {
    do_enable_interrupts(ptr);
}

/**
 * External methods might enable (or disable)
 * interrupts regardless of the TOS interrupts
 * status.
 *
 * This method enables or disables the interrupts
 * depending on the TOS interrupt status.
 */
inline void refresh_interrupts() {
    if (global::disable_depth > 0) {
        platform::disable_interrupts();
    } else {
        platform::enable_interrupts();
    }
}
} // namespace kern

struct no_interrupts {
private:
    no_interrupts() = default;
    friend struct int_guard;
    friend struct int_ctx;
};

/**
 * This type implements a scoped interrupt disable
 * mechanism.
 *
 * Disabling interrupts should be avoided as much
 * as possible, specifically in user code.
 */
struct int_guard : no_interrupts {
public:
    NO_INLINE
    int_guard(void* ptr) {
        kern::disable_interrupts(ptr);
    }
    NO_INLINE
    int_guard() {
        auto ret = __builtin_return_address(0);
        kern::disable_interrupts(ret);
    }
    NO_INLINE
    ~int_guard() {
        auto ret = __builtin_return_address(0);
        kern::enable_interrupts(ret);
    }

    int_guard(int_guard&&) = delete;
};

struct int_ctx : no_interrupts {
    int_ctx() = default;
};
} // namespace tos
