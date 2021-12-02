#pragma once

#include "common/usart.hpp"
#include <arch/drivers.hpp>

namespace tos::bsp {
struct nrf52840_dk_spec {
    static constexpr auto& name() {
        return "nRF52840 Development Kit";
    }

    static constexpr auto led_pin = 13;

    struct xbee_uart {
        static constexpr auto tag = devs::usart<1>;
        static constexpr auto conf = uart::default_38400;
        static constexpr auto tx_pin = 36;
        static constexpr auto rx_pin = 37;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             nrf52::instantiate_pin(rx_pin),
                             nrf52::instantiate_pin(tx_pin));
        }
    };

    struct usart0 {
        static constexpr auto tag = devs::usart<0>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 6;
        static constexpr auto rx_pin = 8;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             nrf52::instantiate_pin(rx_pin),
                             nrf52::instantiate_pin(tx_pin));
        }
    };

    using default_com = usart0;
};
} // namespace tos::bsp