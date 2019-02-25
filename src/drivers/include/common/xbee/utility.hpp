//
// Created by Mehmet Fatih BAKIR on 11/10/2018.
//

#pragma once

#include "types.hpp"
#include <tos/print.hpp>

namespace tos
{
namespace xbee
{
    template <class StreamT>
    void write_addr(StreamT& str, const addr_16& a)
    {
        tos::print(str, uint8_t(a.addr >> 8));
        tos::print(str, uint8_t(a.addr & 0xFF));
    }
}
}
