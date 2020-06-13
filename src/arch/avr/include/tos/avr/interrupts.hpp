#pragma once

#include <avr/interrupt.h>
#include <tos/compiler.hpp>

namespace tos::avr {
ALWAYS_INLINE void enable_interrupts() {
    sei();
}

ALWAYS_INLINE void disable_interrupts() {
    cli();
}
} // namespace tos::avr