//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#pragma once

#include <tos/span.hpp>
#include "gpio.hpp"
#include <drivers/common/i2c.hpp>

namespace tos
{
    namespace nrf52
    {
        class twim
        {
        public:
            twim(gpio::pin_t clock_pin, gpio::pin_t data_pin);

            twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
            twi_tx_res receive(twi_addr_t from, span<char> buf) noexcept;

        private:
        };
    }
}
