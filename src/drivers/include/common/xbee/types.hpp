//
// Created by fatih on 10/10/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
namespace xbee
{
    struct addr_16 { uint16_t addr; };
    struct addr_64;

    struct frame_id_t { uint8_t id; };
    struct length_t { uint16_t len; };

    template <class AddrT>
    class tx_req;

    using tx16_req = tx_req<addr_16>;
    using tx64_req = tx_req<addr_64>;
}
}