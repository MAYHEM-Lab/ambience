//
// Created by Mehmet Fatih BAKIR on 28/04/2018.
//

#pragma once

#include <tos_arch.hpp>
#include <tos/arch.hpp>
#include <stdint.h>

namespace tos {
    namespace kern
    {
        namespace detail {
            extern int8_t disable_depth;
        }

        inline void disable_interrupts()
        {
            if (detail::disable_depth==0) {
                tos_disable_interrupts();
            }
            detail::disable_depth++;
        }

        inline void enable_interrupts()
        {
            detail::disable_depth--;
            if (detail::disable_depth==0) {
                tos_enable_interrupts();
            }
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

    /**
     * This type implements a scoped interrupt disable
     * mechanism.
     *
     * Disabling interrupts should be avoided as much
     * as possible, specifically in user code.
     */
    struct int_guard
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
