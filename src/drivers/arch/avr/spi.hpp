//
// Created by fatih on 4/15/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
namespace avr {
    class spi0 {
    public:
        static void init_master();

        static void init_slave();

        static void enable();

        static void disable();

        static uint8_t exchange(uint8_t byte);

        static void select_slave(uint8_t pin);
        static void deselect_slave(uint8_t pin);
    };
}

struct spi_transaction
{
public:
    explicit spi_transaction(uint8_t pin) : m_omit{false}, m_pin{pin} {
        avr::spi0::select_slave(pin);
    }

    ~spi_transaction()
    {
        if (!m_omit)
        {
            avr::spi0::deselect_slave(m_pin);
        }
    }

    spi_transaction(spi_transaction&& rhs) noexcept : m_omit(false)
    {
        rhs.m_omit = true;
    }

    uint8_t exchange(uint8_t byte)
    {
        return avr::spi0::exchange(byte);
    }

    spi_transaction(const spi_transaction&) = delete;
    spi_transaction& operator=(const spi_transaction&) = delete;
    spi_transaction& operator=(spi_transaction&&) = delete;
private:
    bool m_omit;
    uint8_t m_pin;
};
}