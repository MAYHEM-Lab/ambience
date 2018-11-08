//
// Created by fatih on 11/8/18.
//

#pragma once

#include <drivers/common/i2c.hpp>
#include <drivers/arch/avr/gpio.hpp>
#include <tos/span.hpp>

namespace tos
{
namespace avr
{
    class twim
    {
    public:
        twim(gpio::pin_type clock_pin, gpio::pin_type data_pin);

        twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
        twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;

        twim* operator->() { return this; }
        twim&operator*() { return *this; }
    private:
    };
}
}
