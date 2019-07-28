//
// Created by Mehmet Fatih BAKIR on 28/04/2018.
//

#pragma once

#include <tos/arch.hpp>
#include <cstdint>
#include <tos/barrier.hpp>

namespace tos {
    namespace kern
    {
        namespace detail {
            extern int8_t disable_depth;
        }

        inline void disable_interrupts()
        {
            tos::detail::memory_barrier_enter();
            if (detail::disable_depth==0) {
                tos_disable_interrupts();
            }
            detail::disable_depth++;
            tos::detail::memory_barrier_exit();
        }

		/**
		 * Decrements the interrupt disable count and if
		 * it reaches zero, globally enables interrupts.
		 *
		 * Must be matched by a previous `disable_interrupts` call.
		 */
        inline void enable_interrupts()
        {
            tos::detail::memory_barrier_enter();
            detail::disable_depth--;
            if (detail::disable_depth==0) {
                tos_enable_interrupts();
            }
            tos::detail::memory_barrier_exit();
        }

        /**
         * External methods might enable (or disable)
         * interrupts regardless of the TOS interrupts
         * status.
         *
         * This method enables or disables the interrupts
         * depending on the TOS interrupt status.
         */
        inline void refresh_interrupts()
        {
            if (detail::disable_depth > 0) {
                tos_disable_interrupts();
            }
            else
            {
                tos_enable_interrupts();
            }
        }
    }

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
    struct int_guard : no_interrupts
    {
    public:
        int_guard()
        {
            kern::disable_interrupts();
        }

        ~int_guard()
        {
            kern::enable_interrupts();
        }

        int_guard(int_guard&&) = delete;
    };
}
