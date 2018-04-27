//
// Created by Mehmet Fatih BAKIR on 21/04/2018.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>

namespace tos
{
    enum class usart_modes : uint8_t {
        async = 0,
        sync = 0b01,
        reserved = 0b10,
        spi_master = 0b11
    };

    enum class usart_parity : uint8_t {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    enum class usart_stop_bit : uint8_t {
        one = 0b0,
        two = 0b1
    };

    struct usart_baud_rate
    {
        uint32_t rate;
    };

    namespace tos_literals
    {
        constexpr usart_baud_rate operator""_baud_rate(unsigned long long x)
        {
            return {uint32_t(x)};
        }
    }

    namespace devs
    {
        template <int N> using usart_t = dev<struct _usart_t, N>;
        template <int N> static constexpr usart_t<N> usart{};
    }
}