#pragma once

#include <arch/drivers.hpp>

namespace tos::bsp {
struct nucleo_l412_spec {
    static constexpr auto& name() {
        return "STM32L412 Nucleo";
    }

    static constexpr auto led_pin = 29;

    struct usart1 {
        static constexpr auto tag = devs::usart<1>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 9;
        static constexpr auto rx_pin = 10;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(tx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    struct lpuart1 {
        static constexpr auto tag = devs::lpuart<1>;
        static constexpr auto conf = uart::default_115200;
        static constexpr auto tx_pin = 2;
        static constexpr auto rx_pin = 3;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(tx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    using default_com = lpuart1;
};
} // namespace tos::bsp