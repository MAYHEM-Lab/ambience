#pragma once

#include <arch/drivers.hpp>

namespace tos::bsp {
struct nucleo_l476_spec {
    static constexpr auto& name() {
        return "STM32L476 Nucleo";
    }

    static constexpr auto led_pin = 5;

    struct usart2 {
        static constexpr auto tag = devs::usart<2>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 2;   // PA2
        static constexpr auto rx_pin = 3;   // PA3

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(tx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    struct spi1 {
        static constexpr auto tag = devs::spi<1>;
        static constexpr auto clk_pin = 19;     // PB3
        static constexpr auto miso_pin = 20;    // PB4
        static constexpr auto mosi_pin = 21;    // PB5

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin));
        }
    };

    struct spi1s {
        static constexpr auto tag = devs::spi<1>;
        static constexpr auto clk_pin = 19;     // PB3
        static constexpr auto miso_pin = 20;    // PB4
        static constexpr auto mosi_pin = 21;    // PB5

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin),
                             tos::spi_mode::slave);
        }
    };

    using default_com = usart2;
};
} // namespace tos::bsp