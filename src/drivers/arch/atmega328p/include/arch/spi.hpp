//
// Created by fatih on 4/15/18.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/spi.hpp>
#include <cstdint>
#include <tos/devices.hpp>
#include <tos/span.hpp>

namespace tos {
namespace avr {
/**
 * This class implements the AVR spi hardware.
 *
 * It's one of the first drivers implemented in tos, so it's
 * not as nice as I'd like it to be and is probably in need
 * for a rehaul, specifically, it has both master and slave
 * interfaces for SPI, only one of which can be used at one
 * time.
 *
 * Also, it does not support setting the clock speed for now.
 */
class spi0 : public self_pointing<spi0> {
public:
    using gpio_type = avr::gpio;

    static void init_master();

    static void init_slave();

    static void enable();

    static void disable();

    static expected<void, int> exchange(tos::span<uint8_t> buffer);

    static void select_slave(pin_t pin);

    static void deselect_slave(pin_t pin);

    spi0() = default;
};
} // namespace avr

inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::master_t /*type*/) {
    avr::spi0::init_master();
    return {};
}

inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::slave_t /*type*/) {
    avr::spi0::init_slave();
    return {};
} // namespace avr
} // namespace tos