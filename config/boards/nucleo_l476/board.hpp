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
        static constexpr auto tx_pin = 2;
        static constexpr auto rx_pin = 3;

        static auto open() {
            return tos::open(tag,
                             std::move(conf),
                             stm32::instantiate_pin(tx_pin),
                             stm32::instantiate_pin(tx_pin));
        }
    };

    using default_com = usart2;
};
} // namespace tos::bsp