//
// Created by fatih on 7/12/18.
//

#pragma once

#include <tos/compiler.hpp>

namespace tos {
namespace debug {
/**
 * This function template prevents the compiler from optimizing out the given argument.
 */
template <class Tp>
ALWAYS_INLINE void do_not_optimize(Tp const& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

/**
 * This function template prevents the compiler from optimizing out the given argument.
 */
template <class Tp>
ALWAYS_INLINE void do_not_optimize(Tp& value) {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}
} // namespace debug
} // namespace tos

#if defined(TOS_ARCH_esp8266)
extern "C" int ets_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
#define tos_debug_print
#endif

#if defined(TOS_ARCH_nrf52)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_stm32_hal)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_x86_hosted)
#include <stdio.h>
#define tos_debug_print printf
#endif

#if defined(TOS_ARCH_atmega)
#define tos_debug_print(...)
#endif

#if defined(TOS_ARCH_cc32xx)
#define tos_debug_print(...)
#endif

#if !defined(tos_debug_print)
#endif

#define Expects(x)
#define Ensures(x)