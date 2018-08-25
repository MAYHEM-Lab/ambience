//
// Created by fatih on 6/14/18.
//

#pragma once

#include <stdint.h>

namespace tos
{
    enum class twi_tx_res
    {
        ok = 0,
        addr_nack = 1,
        data_nack = 2,
        other = 3
    };

    enum class twi_rx_res
    {
        ok = 0,
        other = 1
    };

    struct twim_data_rate
    {

    };

    struct twi_addr_t
    {
        uint8_t addr : 7;
    };
}
