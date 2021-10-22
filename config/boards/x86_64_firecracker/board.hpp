#pragma once

#include <tos/peripheral/uart_16550.hpp>

namespace tos::bsp {
struct x86_64_firecracker_spec {
    static constexpr auto& name() {
        return "x86_64 PC";
    }

    struct std_out {
        static auto open() {
            return force_get(tos::x86_64::uart_16550::open());
        }
    };

    using default_com = std_out;
};
} // namespace tos::bsp