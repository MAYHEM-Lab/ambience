#pragma once

#include <tos/x86_64/pit.hpp>

struct timer
    : tos::x86_64::pit
    , tos::self_pointing<timer> {
    timer() {
        ensure(tos::platform::take_irq(0));
        tos::platform::set_irq(0, tos::mem_function_ref<&timer::irq>(*this));
    }

    timer(const timer&) = delete;
    timer(timer&&) = delete;

    void enable() {
        //            tos::x86_64::pic::enable_irq(0);
    }

    void disable() {
        //            tos::x86_64::pic::disable_irq(0);
    }

    void set_callback(tos::function_ref<void()> cb) {
        m_cb = cb;
    }

    void irq(tos::x86_64::exception_frame* frame, int) {
        m_cb();
    }

    tos::function_ref<void()> m_cb{[](void*) {}};
};