#pragma once

#include <cstdint>

namespace tos::raspi3 {
struct exception_table {
    uintptr_t reset;
    uintptr_t undefined_instruction;
    uintptr_t software_interrupt;
    uintptr_t prefetch_abort;
    uintptr_t data_abort;
    uintptr_t _reserved;
    uintptr_t irq;
    uintptr_t fiq;
};
} // namespace tos::raspi3