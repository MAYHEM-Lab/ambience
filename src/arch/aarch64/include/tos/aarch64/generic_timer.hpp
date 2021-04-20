#pragma once

#include <tos/aarch64/assembly.hpp>

namespace tos::aarch64 {
/*
 * This class implements the timer functionality of the arm generic timer.
 *
 * Since this is at the core level, how its interrupts are handled depends on the
 * platform, and therefore, this class is meant to be used in the driver implementation
 * of a platform rather than being directly used.
 *
 * There exists 1 of these timers per core, and they are not memory mapped. This means
 * that these timers cannot be shared across cores.
 */
class generic_timer {
public:
    static uint64_t get_counter() {
        return get_cntpct_el0();
    }

    static uint64_t get_frequency() {
        return get_cntfrq_el0();
    }

    static void set_timeout(uint32_t ticks) {
        set_cntp_tval_el0(ticks);
    }

    static void enable() {
        set_cntp_ctl_el0(1);
    }

    static void disable() {
        set_cntp_ctl_el0(0);
    }
};
} // namespace tos::aarch64