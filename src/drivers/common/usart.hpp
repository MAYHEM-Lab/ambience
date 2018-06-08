//
// Created by Mehmet Fatih BAKIR on 21/04/2018.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>

namespace tos
{
    enum class usart_parity : uint8_t {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    namespace usart
    {
        struct stop_bit_1_t {};
        struct stop_bit_2_t {};

        static constexpr stop_bit_1_t stop_bit_1{};
        static constexpr stop_bit_2_t stop_bit_2{};
    }

    enum class usart_stop_bit : uint8_t {
        one = 0b0,
        two = 0b1
    };

    struct usart_baud_rate
    {
        uint32_t rate;
    };

    template <uint8_t req = 0>
    class usart_config_t
    {
        enum
        {
            baud = 1, parity = 2, stop = 4
        };
    public:

        constexpr usart_config_t<req | baud>
        set(usart_baud_rate br) &&
        {
            return { br, m_par, m_sb };
        }

        constexpr usart_config_t<req | parity>
        set(usart_parity par) &&
        {
            return { m_br, par, m_sb };
        }

        constexpr usart_config_t<req | stop>
        set(usart_stop_bit sb) &&
        {
            return { m_br, m_par, sb };
        }

        usart_baud_rate m_br;
        usart_parity m_par;
        usart_stop_bit m_sb;
    };

    usart_config_t<0> usart_config() { return {}; }

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