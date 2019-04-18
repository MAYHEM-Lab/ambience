//
// Created by fatih on 6/14/18.
//

#pragma once

#include <stdint.h>
#include <tos/span.hpp>

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
        ok,
        addr_nack,
        data_nack,
        other
    };

    struct twim_data_rate
    {

    };

    struct twi_addr_t
    {
        uint8_t addr : 7;
    };

    template <class I2C>
    bool scan_address(I2C& dev, twi_addr_t addr)
    {
        auto res = dev->transmit(addr, empty_span<char>());
        return res == twi_tx_res::ok;
    }
}
