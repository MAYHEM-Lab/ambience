//
// Created by Mehmet Fatih BAKIR on 13/06/2018.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <tos/span.hpp>

#include <tos/semaphore.hpp>
#include <nrfx_twim.h>

namespace tos {
namespace nrf52 {
class twim : public self_pointing<twim>, public non_copy_movable {
public:
    twim(gpio::pin_type clock_pin, gpio::pin_type data_pin);

    twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;
    twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;

private:

    nrfx_twim_evt_type_t m_event;
    tos::semaphore m_event_sem{0};
};
} // namespace nrf52

inline nrf52::twim open_impl(devs::i2c_t<0>,
                      i2c_type::master_t,
                      nrf52::gpio::pin_type scl,
                      nrf52::gpio::pin_type sda) {
    return nrf52::twim{scl, sda};
}

} // namespace tos
