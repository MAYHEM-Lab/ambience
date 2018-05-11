//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#include <eeprom.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

namespace tos
{
    namespace avr
    {
        size_t eeprom::read(uint64_t iop, void* buf, uint16_t bufsz)
        {
            eeprom_read_block(buf, (const void*)iop, bufsz);
            return bufsz;
        }

        void eeprom::write(uint64_t iop, const void* buf, uint16_t bufsz)
        {
            eeprom_update_block(buf, (void*)iop, bufsz);
        }
    }
}
