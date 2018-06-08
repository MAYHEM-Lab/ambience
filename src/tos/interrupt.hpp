//
// Created by Mehmet Fatih BAKIR on 28/04/2018.
//

#pragma once

#include <tos_arch.hpp>
#include <tos/arch.hpp>
#include <stdint.h>

namespace tos {
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

    struct int_guard
    {
    public:
        int_guard()
        {
            disable_interrupts();
        }

        ~int_guard()
        {
            enable_interrupts();
        }

        int_guard(int_guard&&) = delete;
    };
}
