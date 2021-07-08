//
// Created by fatih on 10/16/19.
//

#pragma once

#include <arch/gpio.hpp>
#include <optional>
#include <stm32_hal/gpio.hpp>
#include <tuple>

namespace tos::stm32::detail {
inline namespace l4 {
struct afio {
    static constexpr auto get_usart_afio(int usart,
                                         std::optional<gpio::pin_type> rx,
                                         std::optional<gpio::pin_type> tx) {
#if defined(LPUART1_BASE)
        if (usart == LPUART1_BASE) {
            return std::make_pair(rx ? GPIO_AF8_LPUART1 : -1, tx ? GPIO_AF8_LPUART1 : -1);
        }
#endif
#if defined(UART4_BASE)
        if (usart == UART4_BASE) {
            return std::make_pair(rx ? GPIO_AF8_UART4 : -1, tx ? GPIO_AF8_UART4 : -1);
        }
#endif
        return std::make_pair(rx ? GPIO_AF7_USART1 : -1, tx ? GPIO_AF7_USART1 : -1);
    }

    static constexpr auto
    get_spi_afio(int spi,
                 [[maybe_unused]] const gpio::pin_type& sck,
                 [[maybe_unused]] std::optional<gpio::pin_type> miso,
                 [[maybe_unused]] std::optional<gpio::pin_type> mosi) {

#ifdef SPI3_BASE
        if (spi == SPI3_BASE) {
            return std::make_tuple(GPIO_AF6_SPI3, GPIO_AF6_SPI3, GPIO_AF6_SPI3);
        }
#endif
        return std::make_tuple(GPIO_AF5_SPI1, GPIO_AF5_SPI1, GPIO_AF5_SPI1);
    }
};
} // namespace l4
} // namespace tos::stm32::detail
