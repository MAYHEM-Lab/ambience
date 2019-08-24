//
// Created by fatih on 11/8/18.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <tos/span.hpp>

namespace tos {
namespace avr {
class twim : public self_pointing<twim> {
public:
    twim(gpio::pin_type clock_pin, gpio::pin_type data_pin);

    twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
    twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;
};
} // namespace avr

inline avr::twim open_impl(devs::i2c_t<0>,
                    i2c_type::master_t,
                    avr::gpio::pin_type scl,
                    avr::gpio::pin_type sda) {
    return avr::twim{scl, sda};
}

} // namespace tos
