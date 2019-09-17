//
// Created by Mehmet Fatih BAKIR on 21/04/2018.
//

#pragma once

#include <stdint.h>
#include <tos/ct_map.hpp>
#include <tos/devices.hpp>

namespace tos {
enum class usart_parity : uint8_t
{
    disabled = 0,
    reserved = 0b01,
    even = 0b10,
    odd = 0b11
};

enum class usart_stop_bit : uint8_t
{
    one = 0b0,
    two = 0b1
};

struct usart_baud_rate {
    uint32_t rate;
};

namespace uart {
/**
 * This struct template is used to pass and store
 * the receive and transmit pins of a UART peripheral.
 * @tparam PinT type of the pins
 */
template<class PinT>
struct rx_tx_pins {
    PinT rx, tx;
};

template<class PinT>
rx_tx_pins(const PinT&, const PinT&)->rx_tx_pins<PinT>;
} // namespace uart

template<class...>
struct pair_t {};
struct usart_key_policy {
    static constexpr auto m =
        tos::make_map()
            .add<pair_t<usart_baud_rate, usart_baud_rate>>(std::true_type{})
            .add<pair_t<usart_parity, usart_parity>>(std::true_type{})
            .add<pair_t<usart_stop_bit, usart_stop_bit>>(std::true_type{});

    template<class KeyT, class ValT>
    static constexpr auto validate(ct<KeyT>, ct<ValT>) {
        constexpr auto x = std::false_type{};
        return get_or<pair_t<KeyT, ValT>>(x, m);
    }
};

constexpr ct_map<usart_key_policy> usart_config() {
    return ct_map<usart_key_policy>{};
}

namespace uart {
static constexpr auto default_9600 = usart_config()
                                         .add(tos::usart_baud_rate{9600})
                                         .add(tos::usart_parity::disabled)
                                         .add(tos::usart_stop_bit::one);
} // namespace uart

namespace tos_literals {
constexpr usart_baud_rate operator""_baud_rate(unsigned long long x) {
    return {uint32_t(x)};
}
} // namespace tos_literals

namespace devs {
template<int N>
using usart_t = dev<struct _usart_t, N>;
template<int N>
static constexpr usart_t<N> usart{};
} // namespace devs

struct any_usart {
    virtual int write(tos::span<const char>) = 0;
    virtual tos::span<char> read(tos::span<char>) = 0;
    virtual ~any_usart() = default;
};

namespace detail {
template<class BaseUsartT>
class erased_usart : public any_usart {
public:
    erased_usart(const BaseUsartT& usart)
        : m_impl{usart} {
    }

    erased_usart(BaseUsartT&& usart)
        : m_impl{std::move(usart)} {
    }

    int write(tos::span<const char> span) override {
        return m_impl->write(span);
    }

    span<char> read(tos::span<char> span) override {
        return m_impl->read(span);
    }

private:
    BaseUsartT m_impl;
};

class null_usart : public any_usart {
public:
    int write(tos::span<const char> span) override { return 0; }
    span<char> read(tos::span<char> span) override { return tos::span<char>(nullptr); }
};
} // namespace detail

template<class UsartT>
auto erase_usart(UsartT&& usart) -> detail::erased_usart<UsartT> {
    return {std::forward<UsartT>(usart)};
}
} // namespace tos