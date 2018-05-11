//
// Created by Mehmet Fatih BAKIR on 11/05/2018.
//

#pragma once

#include <tos/devices.hpp>

namespace tos
{
    namespace devs
    {
        template <uint8_t N>
        using eeprom_t = dev<struct _eeprom_t, N>;

        template <uint8_t N>
        static constexpr eeprom_t<N> eeprom{};
    }
}
