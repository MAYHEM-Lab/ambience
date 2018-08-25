//
// Created by fatih on 8/24/18.
//

#pragma once

#include <drivers/common/i2c.hpp>
#include <tos/span.hpp>
#include "gpio.hpp"

namespace tos
{
    namespace esp82
    {
        class twim
        {
        public:
            twim(pin_t clock_pin, pin_t data_pin);

            twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
            twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;
        };
    }
}