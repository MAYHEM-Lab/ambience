//
// Created by fatih on 6/14/18.
//

#pragma once

namespace tos
{
    enum class twi_tx_res
    {
        ok = 0,
        addr_nack = 1,
        data_nack = 2,
        other = 3
    };

    struct twi_addr_t
    {
        uint8_t addr : 7;
    };
}
