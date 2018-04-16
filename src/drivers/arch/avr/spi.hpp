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

struct spi_guard
{
public:
    spi_guard() {
        begin_spi_transaction();
    }
    ~spi_guard()
    {
        end_spi_transaction();
    }

    spi_guard(const spi_guard&) = delete;
    spi_guard(spi_guard&&) = delete;
    spi_guard& operator=(const spi_guard&) = delete;
    spi_guard& operator=(spi_guard&&) = delete;
};

}
}