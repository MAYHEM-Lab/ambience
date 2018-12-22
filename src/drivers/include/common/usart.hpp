//
// Created by Mehmet Fatih BAKIR on 21/04/2018.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>
#include <tos/ct_map.hpp>

namespace tos
{
    enum class usart_parity : uint8_t {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    namespace uart
    {
        struct stop_bit_1_t {};
        struct stop_bit_2_t {};

        static constexpr stop_bit_1_t stop_bit_1{};
        static constexpr stop_bit_2_t stop_bit_2{};
    } // namespace uart

    namespace uart
    {
        namespace events
        {
            static constexpr struct sent_t{} sent{};
            static constexpr struct recv_t{} recv{};
        } // namespace events
    } // namespace uart

    enum class usart_stop_bit : uint8_t {
        one = 0b0,
        two = 0b1
    };

    struct usart_baud_rate
    {
        uint32_t rate;
    };

    template <class...> struct pair_t {};
    struct usart_key_policy
    {
        static constexpr auto m = tos::make_map()
                .add<pair_t<usart_baud_rate, usart_baud_rate>>(std::true_type{})
                .add<pair_t<usart_parity, usart_parity>>(std::true_type{})
                .add<pair_t<usart_stop_bit, usart_stop_bit>>(std::true_type{});

        template <class KeyT, class ValT>
        static constexpr auto validate(ct<KeyT>, ct<ValT>) {
            constexpr auto x = std::false_type{};
            return get_or<pair_t<KeyT, ValT>>(x, m);
        }
    };

    constexpr inline ct_map<usart_key_policy> usart_config() { return ct_map<usart_key_policy>{}; }

	namespace uart
	{
		static constexpr auto default_9600 = usart_config()
			.add(tos::usart_baud_rate{ 9600 })
			.add(tos::usart_parity::disabled)
			.add(tos::usart_stop_bit::one);
	} // namespace uart

    namespace tos_literals
    {
        constexpr usart_baud_rate operator""_baud_rate(unsigned long long x)
        {
            return {uint32_t(x)};
        }
    } // namespace tos_literals

    namespace devs
    {
        template <int N> using usart_t = dev<struct _usart_t, N>;
        template <int N> static constexpr usart_t<N> usart{};
    } // namespace devs
} // namespace tos