//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#pragma once

#include <tos/span.hpp>
#include "gpio.hpp"

namespace tos
{
    namespace nrf52
    {
        enum class twi_tx_res
        {
            ok,
            addr_nack,
            data_nack
        };

        struct twi_addr_t
        {
            uint8_t addr : 7;
        };

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
