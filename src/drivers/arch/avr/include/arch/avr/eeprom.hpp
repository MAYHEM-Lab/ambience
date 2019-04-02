//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <common/eeprom.hpp>
#include <common/driver_base.hpp>
#include <tos/span.hpp>

namespace tos
{
    namespace avr
    {
        class eeprom : public self_pointing<eeprom>
        {
        public:
            tos::span<char> read(uint64_t iop, tos::span<char> buf);
            size_t write(uint64_t iop, tos::span<const char> data);
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
        inline tos::span<char> eeprom::read(uint64_t iop, tos::span<char> buf)
        {
            eeprom_read_block(buf.data(), (const void*)iop, buf.size());
            return buf;
        }

        inline size_t eeprom::write(uint64_t iop, tos::span<const char> data)
        {
            eeprom_update_block(data.data(), (void*)iop, data.size());
            return data.size();
        }
    }
}
