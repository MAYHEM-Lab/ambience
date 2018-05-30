//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <drivers/common/eeprom.hpp>

namespace tos
{
    namespace avr
    {
        class eeprom
        {
        public:
            size_t read(uint64_t iop, void* buf, uint16_t bufsz);
            void write(uint64_t iop, const void* buf, uint16_t bufsz);
        };
    }

    inline avr::eeprom* open_impl(devs::eeprom_t<0>)
    {
        return nullptr;
    }
}
