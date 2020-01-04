//
// Created by fatih on 12/30/19.
//

#include <arch/detail/events.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/semaphore.hpp>

namespace tos::cc32xx {
inline tos::basic_fixed_fifo<tos::cc32xx::events, 20, tos::ring_buf> evq;
}

extern "C" {
inline tos::semaphore loop_sem{0};
}
