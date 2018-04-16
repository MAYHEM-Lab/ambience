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

void spi_write_byte(uint8_t);
uint8_t spi_read_byte();
}
}