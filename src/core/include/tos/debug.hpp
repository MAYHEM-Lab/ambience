//
// Created by fatih on 7/12/18.
//

#pragma once

#include <tos/arch.hpp>

namespace tos
{
    template <class ErrT>
    [[noreturn]]
    void raise(ErrT&&);

    namespace kern
    {
        /**
         * This function is used to signal a non-recoverable fault in
         * the system.
         *
         * The execution of the whole operating system will be suspended.
         *
         * In a debug environment, the OS will idle forever.
         *
         * In a release environemnt, if possible, the incident will be
         * logged and the system will be rebooted.
         *
         * Should be used only for fatal errors such as broken invariants.
         *
         * @tparam ErrT an explanation for the crash
         */
        template <class ErrT>
        [[noreturn]]
        void fatal(ErrT&&) noexcept {
            // or directly reset?
            tos_force_reset();
            //tos::this_thread::block_forever();
        }
    }
}

#if defined(TOS_ARCH_esp8266)
extern "C" int ets_printf(const char *format, ...)  __attribute__ ((format (printf, 1, 2)));
#define tos_debug_print ets_printf
#endif

#if defined(TOS_ARCH_nrf52)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_stm32)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_x86)
#include <stdio.h>
#define tos_debug_print printf
#endif

#if defined(TOS_ARCH_atmega)
#define tos_debug_print(...)
#endif

#if !defined(tos_debug_print)
#endif

#define Expects(x)
#define Ensures(x)