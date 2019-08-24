//
// Created by fatih on 8/24/18.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <tos/span.hpp>

namespace tos {
namespace esp82 {
/**
 * The ESP8266 does not have a hardware I2C peripheral. This class
 * implements a software based bit-banged I2C driver.
 */
class twim : public self_pointing<twim> {
public:
    twim(pin_t clock_pin, pin_t data_pin);

    twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
    twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;
};
} // namespace esp82

inline esp82::twim open_impl(devs::i2c_t<0>,
                      i2c_type::master_t,
                      esp82::gpio::pin_type scl,
                      esp82::gpio::pin_type sda) {
    return esp82::twim{scl, sda};
}

} // namespace tos