//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>
#include <common/gpio.hpp>
#include <tos/expected.hpp>
#include <tos/span.hpp>

namespace tos
{
    struct moved_flag
    {
        moved_flag() = default;
        moved_flag(moved_flag&& rhs) noexcept { rhs.m_moved = true; }

        explicit operator bool() const { return m_moved; }

        moved_flag& operator=(moved_flag&& rhs) = delete;

    private:
        bool m_moved = false;
    };

    namespace spi {
    template <class SpiT>
    auto exchange(SpiT& spi, uint8_t val)
    {
        using SpiRetT = decltype(spi->exchange(tos::monospan(val)));
        using ErrT = typename SpiRetT::error_type;
        using RetT = expected<uint8_t, ErrT>;
        auto res = spi->exchange(tos::monospan(val));
        if (!res)
        {
            return RetT(unexpected(force_error(res)));
        }
        return RetT(val);
    }
    }


    template<class T>
    struct spi_transaction
    {
    public:
        explicit spi_transaction(T& spi, typename T::gpio_type& gpio, typename T::gpio_type::pin_type pin)
                : m_pin{pin}, m_g{gpio}, m_spi{spi}
        {
            m_g->write(m_pin, tos::digital::low);
        }

        ~spi_transaction()
        {
            if (!m_omit) {
                m_g->write(m_pin, tos::digital::high);
            }
        }

        T* operator->() {
            return &m_spi;
        }

        spi_transaction(spi_transaction&& rhs) noexcept = default;

        spi_transaction(const spi_transaction&) = delete;

        spi_transaction& operator=(const spi_transaction&) = delete;

        spi_transaction& operator=(spi_transaction&&) = delete;

    private:
        moved_flag m_omit;
        T& m_spi;
        typename T::gpio_type& m_g;
        typename T::gpio_type::pin_type m_pin;
    };

    struct spi_mode
    {
        struct slave_t {};
        struct master_t {};

        static constexpr slave_t slave{};
        static constexpr master_t master{};
    };

    namespace devs {
        template<int N> using spi_t = dev<struct _spi_t, N>;
        template<int N> static constexpr spi_t<N> spi{};
    }
}