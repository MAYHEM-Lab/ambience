#pragma once

#include "scrollable.hpp"
#include <tos/peripheral/uart_16550.hpp>

namespace tos::bsp {
struct digitalocean_vm_spec {
    static constexpr auto& name() {
        return "DigitalOcean VM";
    }

    struct std_out {
        static auto open() {
            /*void kb_isr(tos::x86_64::exception_frame* frame, int) {
                auto kc = tos::x86_64::port(0x60).inb();
                if (kc == 36) { // j
                    vga.scroll_up();
                } else if (kc == 37) { // k
                    vga.scroll_down();
                }

                ensure(tos::platform::take_irq(1));
                tos::platform::set_irq(1, tos::free_function_ref(+kb_isr));
            }*/
//            return force_get(tos::x86_64::uart_16550::open());

            return scrollable{};
        }
    };

    using default_com = std_out;
};
} // namespace tos::bsp