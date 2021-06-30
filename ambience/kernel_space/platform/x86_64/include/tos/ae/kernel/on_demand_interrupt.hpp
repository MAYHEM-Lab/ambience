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
        auto post = [&](auto&&... args) {
            tos::platform::reset_post_irq();
            t();
        };
        auto h = [&](auto&&... args) {
            tos::platform::set_post_irq(
                tos::function_ref<void(tos::x86_64::exception_frame*)>(post));
        };
        tos::platform::set_irq(irq, tos::platform::irq_handler_t(h));
        tos::x86_64::int0x2c();
    }
};
