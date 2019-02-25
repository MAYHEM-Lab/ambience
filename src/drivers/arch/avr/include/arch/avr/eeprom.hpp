//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <common/eeprom.hpp>

namespace tos
{
    namespace avr
    {
        class eeprom
        {
        public:
            size_t read(uint64_t iop, void* buf, uint16_t bufsz);
            void write(uint64_t iop, const void* buf, uint16_t bufsz);

            eeprom*operator->(){ return this; }
            eeprom&operator*(){ return *this; }
        };
    }

    inline avr::eeprom open_impl(devs::eeprom_t<0>)
    {
        return {};
    }
}

// impl

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

namespace tos
{
    namespace avr
    {
        inline size_t eeprom::read(uint64_t iop, void* buf, uint16_t bufsz)
        {
            eeprom_read_block(buf, (const void*)iop, bufsz);
            return bufsz;
        }

        inline void eeprom::write(uint64_t iop, const void* buf, uint16_t bufsz)
        {
            eeprom_update_block(buf, (void*)iop, bufsz);
        }
    }
}
