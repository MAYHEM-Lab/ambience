#pragma once

#include <arch/drivers.hpp>

namespace tos::bsp {
struct l4_iot_disco_spec {
    static constexpr auto& name() {
        return "STM32 IoT Demo Board";
    }

    static constexpr auto led_pin = 5;

    struct usart1 {
        static constexpr auto tag = devs::usart<1>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 22;
        static constexpr auto rx_pin = 23;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(tx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    using default_com = usart1;

    struct spi1 {
        static constexpr auto tag = devs::spi<1>;
        static constexpr auto clk_pin = 5;
        static constexpr auto miso_pin = 6;
        static constexpr auto mosi_pin = 7;

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin));
        }
    };

    struct spi3 {
        static constexpr auto tag = devs::spi<3>;
        static constexpr auto clk_pin = 42;
        static constexpr auto miso_pin = 43;
        static constexpr auto mosi_pin = 44;

        static auto open() {
            return tos::open(tag,
                             stm32::instantiate_pin(clk_pin),
                             stm32::instantiate_pin(miso_pin),
                             stm32::instantiate_pin(mosi_pin));
        }
    };

    struct ble {
        using spi_dev = spi3;
        static constexpr auto exti_pin = 70;
        static constexpr auto cs_pin = 61;
        static constexpr auto reset_pin = 8;
    };
};
} // namespace tos::bsp