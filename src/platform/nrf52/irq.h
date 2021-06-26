#pragma once

extern "C" {
#define IRQ(x) void x();
#include "nrf52_irq.inc"
#undef IRQ
}

namespace tos::arm {
enum class external_interrupts
{
#define IRQ(x) x,
#include "nrf52_irq.inc"
#undef IRQ
    size
};
} // namespace tos::arm
