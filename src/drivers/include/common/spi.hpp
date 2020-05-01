//
// Created by fatih on 4/16/18.
//

#pragma once

#include <common/driver_base.hpp>
#include <common/gpio.hpp>
#include <cstdint>
#include <tos/devices.hpp>
#include <tos/expected.hpp>
#include <tos/moved_flag.hpp>
#include <tos/span.hpp>
#include <type_traits>

namespace tos {
namespace spi {
template<class SpiT>
auto exchange(SpiT& spi, uint8_t val) {
    uint8_t rx{0xFF};
    using SpiRetT = decltype(spi->exchange(tos::monospan(rx), tos::monospan(val)));
    using ErrT = typename SpiRetT::error_type;
    using RetT = expected<uint8_t, ErrT>;
    auto res = spi->exchange(tos::monospan(rx), tos::monospan(val));
    if (!res) {
        return RetT(unexpected(force_error(res)));
    }
    return RetT(rx);
}
} // namespace spi


template<class T>
struct spi_transaction {
public:
    using gpio_type = typename std::remove_pointer_t<T>::gpio_type;
    explicit spi_transaction(T spi, gpio_type* gpio, typename gpio_type::pin_type pin)
        : m_pin{pin}
        , m_g{gpio}
        , m_spi{spi} {
        m_g->write(m_pin, tos::digital::low);
    }

    ~spi_transaction() {
        if (!m_omit) {
            m_g->write(m_pin, tos::digital::high);
        }
    }

    T& operator->() {
        return m_spi;
    }

    spi_transaction(spi_transaction&& rhs) noexcept = default;

    spi_transaction(const spi_transaction&) = delete;

    spi_transaction& operator=(const spi_transaction&) = delete;

    spi_transaction& operator=(spi_transaction&&) = delete;

private:
    typename gpio_type::pin_type m_pin;
    gpio_type* m_g;
    T m_spi;
    moved_flag m_omit;
};

struct spi_mode {
    struct slave_t {};
    struct master_t {};

    static constexpr slave_t slave{};
    static constexpr master_t master{};
};

namespace devs {
template<int N>
using spi_t = dev<struct _spi_t, N>;
template<int N>
static constexpr spi_t<N> spi{};
} // namespace devs

struct any_spi : public self_pointing<any_spi> {
    virtual void write(span<const char>) = 0;
    virtual void exchange(span<char>, span<const char>) = 0;
    virtual ~any_spi() = default;
};
} // namespace tos