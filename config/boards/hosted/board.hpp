#pragma once

#include <arch/drivers.hpp>

namespace tos::bsp {
struct hosted_spec {
    static constexpr auto& name() {
        return "x86 Hosted Board";
    }

    struct std_out {
        static constexpr auto tag = devs::usart<0>;
        static constexpr auto conf = uart::default_115200;

        static auto open() {
            return tos::open(tag, std::move(conf));
        }
    };

    struct xbee_uart {
        static auto open() {
            return tos::hosted::usart{get_io(), "/dev/ttyUSB1", tos::uart::default_38400};
        }
    };

    using default_com = std_out;
};
} // namespace tos::bsp