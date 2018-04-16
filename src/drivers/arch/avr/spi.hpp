//
// Created by fatih on 4/15/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
namespace avr
{
class spi0
{
public:
    static void init_master();
    static void init_slave();

    static void enable();
    static void disable();
};

uint8_t spi_put_byte(uint8_t b);
void spi_write_byte(uint8_t);
uint8_t spi_read_byte();

void begin_spi_transaction();
void end_spi_transaction();

struct spi_transaction
{
public:
    spi_transaction() : m_omit{false} {
        begin_spi_transaction();
    }

    ~spi_transaction()
    {
        if (!m_omit)
        {
            end_spi_transaction();
        }
    }

    spi_transaction(spi_transaction&& rhs) noexcept : m_omit(false)
    {
        rhs.m_omit = true;
    }

    spi_transaction(const spi_transaction&) = delete;
    spi_transaction& operator=(const spi_transaction&) = delete;
    spi_transaction& operator=(spi_transaction&&) = delete;
private:
    bool m_omit;
};

}
}