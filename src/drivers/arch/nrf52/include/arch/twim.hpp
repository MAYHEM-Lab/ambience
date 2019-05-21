//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#pragma once

#include <tos/span.hpp>
#include "gpio.hpp"
#include <common/i2c.hpp>
#include <common/driver_base.hpp>

namespace tos
{
    namespace nrf52
    {
        class twim : public self_pointing<twim>
        {
        public:
            twim(gpio::pin_type clock_pin, gpio::pin_type data_pin);

            twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
            twi_tx_res receive(twi_addr_t from, span<char> buf) noexcept;
        };
    }
}
