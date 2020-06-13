#pragma once

#include <avr/io.h>
#include <tos/compiler.hpp>

namespace tos::avr {
ALWAYS_INLINE
void set_stack_ptr(char* ptr) {
    SP = reinterpret_cast<uint16_t>(ptr);
}

ALWAYS_INLINE
void* get_stack_ptr() {
    return reinterpret_cast<void*>(SP);
}
} // namespace tos::avr