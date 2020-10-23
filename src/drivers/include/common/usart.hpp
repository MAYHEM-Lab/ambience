//
// Created by Mehmet Fatih BAKIR on 21/04/2018.
//

#pragma once

#include <algorithm>
#include <common/driver_base.hpp>
#include <cstdint>
#include <tos/ct_map.hpp>
#include <tos/devices.hpp>
#include <tos/mutex.hpp>
#include <tos/span.hpp>

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
rx_tx_pins(const PinT&, const PinT&) -> rx_tx_pins<PinT>;
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
static constexpr auto default_115200 = usart_config()
                                           .add(tos::usart_baud_rate{115200})
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
template<int N>
using lpuart_t = dev<struct _lpuart_t, N>;
template<int N>
static constexpr lpuart_t<N> lpuart{};
} // namespace devs

/**
 * This type represents an interface type to any USART in tos.
 *
 * This class is not meant to be inherited by the drivers, but rather, objects
 * that implement this interface should be acquired by _erasing_ the concrete
 * driver using the `tos::erase_usart` function.
 *
 * As it uses virtual functions as the actual driver interface, it'll be less
 * efficient to use this type instead of the concrete object or a function template
 * that's parameterized over the actual USART type.
 */
struct any_usart : public self_pointing<any_usart> {
    virtual int write(tos::span<const uint8_t>) = 0;
    virtual tos::span<uint8_t> read(tos::span<uint8_t>) = 0;
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

    int write(tos::span<const uint8_t> span) override {
        return m_impl->write(span);
    }

    span<uint8_t> read(tos::span<uint8_t> span) override {
        return m_impl->read(span);
    }

private:
    BaseUsartT m_impl;
};
} // namespace detail

/**
 * This class implements a USART driver that discards every piece of
 * data that's passed to it.
 */
class null_usart : public self_pointing<null_usart> {
public:
    int write(tos::span<const uint8_t>) {
        return 0;
    }
    span<uint8_t> read(tos::span<uint8_t>) {
        return tos::span<uint8_t>(nullptr);
    }
};

/**
 * Erases the type of the given concrete USART driver object. Returns
 * an object that implements the `tos::any_usart` interface.
 *
 *      tos::any_usart* output;
 *      auto concrete_driver = tos::open(tos::devs::usart<0>, ...args...);
 *      output = &concrete_driver; // Compilation error!
 *      auto erased_usart = tos::erase_usart(std::move(concrete_driver));
 *      output = &erased_usart; // Ok.
 *
 * This function does not perform any memory allocation. The passed
 * object will be stored in the erased object by value. If you do not
 * wish to transfer ownership of the driver, pass the address of the
 * driver you wish to erase:
 *
 *      auto erased_usart = tos::erase_usart(&concrete_driver);
 *
 * The deleted driver must implement the whole USART driver interface. For
 * instance, USART drivers providing only a write function but no read
 * function cannot be type-erased.
 */
template<class UsartT>
auto erase_usart(UsartT&& usart) -> detail::erased_usart<UsartT> {
    return {std::forward<UsartT>(usart)};
}

template<class T>
class thread_safe_usart : public self_pointing<thread_safe_usart<T>> {
public:
    template <class U, std::enable_if_t<!std::is_same_v<U, thread_safe_usart>>* = nullptr>
    explicit thread_safe_usart(U&& t)
        : m_impl(std::forward<U>(t)) {
    }

    auto write(span<const uint8_t> span) {
        lock_guard lk{m_mutex};
        return m_impl->write(span);
    }

    auto read(span<uint8_t> span) {
        lock_guard lk{m_mutex};
        return m_impl->read(span);
    }

public:
    mutex m_mutex;
    T m_impl;
};

template <class T>
thread_safe_usart(T&&) -> thread_safe_usart<T>;

template<class T, size_t BufferSize = 512>
class buffered_usart : public self_pointing<buffered_usart<T, BufferSize>> {
public:
    template <class U, std::enable_if_t<!std::is_same_v<U, buffered_usart>>* = nullptr>
    explicit buffered_usart(U&& t)
        : m_impl(std::forward<U>(t)) {
    }

    auto write(tos::span<const uint8_t> span) {
        if (m_disabled || (span.size() + m_buf_cur > m_buffer.size())) {
            flush();
            m_impl->write(span);
        } else {
            std::copy_n(span.begin(), span.size(), m_buffer.begin() + m_buf_cur);
            m_buf_cur += span.size();
        }
        return span.size();
    }

    auto read(tos::span<uint8_t> span) {
        return m_impl->read(span);
    }

    ~buffered_usart() {
        flush();
    }

    void flush() {
        if (m_buf_cur == 0) {
            return;
        }
        m_impl->write(tos::span<const uint8_t>(m_buffer).slice(0, m_buf_cur));
        m_buf_cur = 0;
    }

    void disable_buffer() {
        flush();
        m_disabled = true;
    }

    void enable_buffer() {
        m_disabled = false;
    }

private:
    bool m_disabled = false;
    T m_impl;
    std::array<uint8_t, BufferSize> m_buffer;
    uint16_t m_buf_cur = 0;
};

template <class T>
buffered_usart(T&&) -> buffered_usart<T>;
} // namespace tos