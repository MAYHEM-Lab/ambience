#pragma once

#include <tos/platform.hpp>
#include <tos/x86_64/assembly.hpp>

struct on_demand_interrupt {
    int irq;
    on_demand_interrupt() {
        irq = 12;
        ensure(tos::platform::take_irq(irq));
    }
    template<class T>
    void operator()(T&& t) {
        tos::platform::set_irq(irq, tos::platform::irq_handler_t(t));
        tos::x86_64::int0x2c();
    }
};
