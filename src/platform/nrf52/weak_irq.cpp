#include "irq.h"

extern "C" {
void Default_Handler() {
    while (true) {
    }
}

#define IRQ(x) void x() __attribute__((weak, alias("Default_Handler")));
#include "nrf52_irq.inc"
#undef IRQ
}