//
// Created by fatih on 4/17/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    class spi_sd_card
    {
    public:
        explicit spi_sd_card(uint8_t spi_pin);

        bool init();
        void read(void* to, uint32_t blk, uint16_t len, uint16_t offset = 0);

    private:
        uint8_t m_spi_pin;
    };
}