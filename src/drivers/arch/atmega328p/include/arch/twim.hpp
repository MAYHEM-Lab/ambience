//
// Created by fatih on 11/8/18.
//

#pragma once

#include <common/i2c.hpp>
#include "gpio.hpp"
#include <tos/span.hpp>
#include <common/driver_base.hpp>

namespace tos
{
namespace avr
{
    class twim : public self_pointing<twim>
    {
    public:
        twim(gpio::pin_type clock_pin, gpio::pin_type data_pin);

        twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
        twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;
    };
}
}
