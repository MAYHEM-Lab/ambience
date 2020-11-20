#pragma once

#include <avr/interrupt.h>
#include <tos/compiler.hpp>

namespace tos::avr {
inline bool interrupts_disabled() {
    uint8_t sreg;
    asm volatile("in %0, __SREG__" : "=r"(sreg)::"memory");
    return sreg & 0x80;
}

ALWAYS_INLINE void enable_interrupts() {
    sei();
}

ALWAYS_INLINE void disable_interrupts() {
    cli();
}
} // namespace tos::avr