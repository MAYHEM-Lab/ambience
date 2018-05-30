//
// Created by fatih on 5/29/18.
//

#pragma once

#include <tos/ft.hpp>
#include <tos/interrupt.hpp>
#include <tos/compiler.hpp>

extern void tos_main();

namespace tos
{
    void kernel_main() TOS_MAIN NORETURN ALWAYS_INLINE;
    inline void kernel_main()
    {
        tos::enable_interrupts();

        tos_main();

        while (true)
        {
            auto res = tos::schedule();
            if (res == tos::exit_reason::restart) tos_reboot();
            if (res == tos::exit_reason::power_down) tos_power_down();
            if (res == tos::exit_reason::idle);
        }
    }
}