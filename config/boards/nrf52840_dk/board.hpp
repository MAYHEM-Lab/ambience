#pragma once

#include "arch/gpio.hpp"
#include "arch/twim.hpp"
#include "common/i2c.hpp"
#include "common/usart.hpp"
#include <arch/drivers.hpp>
#include <chrono>
#include <nrf_delay.h>
#include <tos/device/bme280.hpp>

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
        static constexpr auto rx_pin = 38;

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

    struct i2c1_master {
        static constexpr auto tag = devs::i2c<0>;
        static constexpr auto clk_pin = 39;
        static constexpr auto data_pin = 40;

        static auto open() {
            return tos::open(tag,
                             i2c_type::master,
                             nrf52::instantiate_pin(clk_pin),
                             nrf52::instantiate_pin(data_pin));
        }
    };

    struct bme280 {
        static auto open() {
            return tos::device::bme280::driver{
                twi_addr_t{BME280_I2C_ADDR_PRIM},
                new nrf52::twim(i2c1_master::open()),
                [](std::chrono::milliseconds ms) { nrf_delay_ms(ms.count()); }};
        }
    };

    using default_com = usart0;
};
} // namespace tos::bsp