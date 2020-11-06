#pragma once

#include <arch/drivers.hpp>

namespace tos::bsp {
struct nrf52_thingy_spec {
    static constexpr auto& name() {
        return "nRF52 Thingy";
    }

    static constexpr auto led_pin = 13;

    struct usart0 {
        static constexpr auto tag = devs::usart<0>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 6;
        static constexpr auto rx_pin = 8;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             nrf52::instantiate_pin(tx_pin),
                             nrf52::instantiate_pin(tx_pin));
        }
    };

    using default_com = usart0;

    struct i2c {
        static constexpr auto tag = devs::i2c<0>;
        static constexpr auto scl_pin = 8;
        static constexpr auto sda_pin = 7;

        static auto open() {
            return tos::open(tag,
                             i2c_type::master,
                             nrf52::instantiate_pin(scl_pin),
                             nrf52::instantiate_pin(sda_pin));
        }
    };

    static constexpr auto button_pin = 11;
    static constexpr auto battery_input = 28;
    static constexpr auto battery_charge_status = 17;

    struct sxio {
        static constexpr auto i2c_addr = 0x3E;
    };
};
} // namespace tos::bsp