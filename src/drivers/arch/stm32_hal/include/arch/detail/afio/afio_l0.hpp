//
// Created by fatih on 10/16/19.
//

#pragma once

#include <arch/gpio.hpp>
#include <optional>
#include <stm32_hal/gpio.hpp>
#include <tuple>

namespace tos::stm32::detail {
inline namespace l0 {
struct afio {
    static constexpr auto get_usart_afio(int,
                                         std::optional<gpio::pin_type> rx,
                                         std::optional<gpio::pin_type> tx) {
        return std::make_pair(rx ? GPIO_AF0_USART1 : -1, tx ? GPIO_AF0_USART1 : -1);
    }

    static constexpr auto get_spi_afio(int spi,
                                       const gpio::pin_type& sck,
                                       std::optional<gpio::pin_type> miso,
                                       std::optional<gpio::pin_type> mosi) {
        return std::make_tuple(
                GPIO_AF0_SPI1,
                GPIO_AF0_SPI1,
                GPIO_AF0_SPI1
        );
    }
};
} // namespace l4
} // namespace tos::stm32::detail