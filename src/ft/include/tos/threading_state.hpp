#pragma once

#include <tos/tcb.hpp>

namespace tos {
struct threading_state {
    kern::tcb* current_thread;
    int8_t num_threads = 0;
};

namespace global {
extern threading_state thread_state;
}

/**
 * Returns a pointer to the currently running thread.
 *
 * Returns `nullptr` if there's no active thread at the moment.
 *
 * @return pointer to the current thread
 */
ALWAYS_INLINE kern::tcb* self() {
    return global::thread_state.current_thread;
}
} // namespace tos