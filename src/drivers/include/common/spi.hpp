//
// Created by fatih on 4/16/18.
//

#pragma once

#include <stdint.h>
#include <tos/devices.hpp>
#include <common/gpio.hpp>

namespace tos
{
    struct spi_interface final
    {
        uint8_t exchange(uint8_t);
        void exchange_many(uint8_t* buffer, uint16_t len);
    };

    template<class T>
    struct spi_transaction
    {
    public:
        explicit spi_transaction(typename T::gpio_type::pin_type pin)
                :m_omit{false}, m_pin{pin}
        {
            T::select_slave(pin);
        }

        ~spi_transaction()
        {
            if (!m_omit) {
                T::deselect_slave(m_pin);
            }
        }

        spi_transaction(spi_transaction&& rhs) noexcept
                :m_omit(false), m_pin{rhs.m_pin}
        {
            rhs.m_omit = true;
        }

        uint8_t exchange(uint8_t byte)
        {
            return T::exchange(byte);
        }

        void exchange_many(uint8_t* buf, uint16_t sz)
        {
            return T::exchange_many({buf, sz});
        }

        spi_transaction(const spi_transaction&) = delete;

        spi_transaction& operator=(const spi_transaction&) = delete;

        spi_transaction& operator=(spi_transaction&&) = delete;

    private:
        bool m_omit;
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