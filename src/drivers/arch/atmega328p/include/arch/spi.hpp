//
// Created by fatih on 4/15/18.
//

#pragma once

#include "gpio.hpp"
#include "tos/utility.hpp"

#include <common/driver_base.hpp>
#include <common/spi.hpp>
#include <cstdint>
#include <tos/devices.hpp>
#include <tos/semaphore.hpp>
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
class spi0
    : public self_pointing<spi0>
    , public tracked_driver<spi0, 1>
    , public non_copy_movable {
public:
    using gpio_type = avr::gpio;

    void init_master();

    void init_slave();

    expected<void, int> exchange(tos::span<uint8_t> rxtx) {
        return exchange(rxtx, rxtx);
    }
    expected<void, int> exchange(tos::span<uint8_t> rx, tos::span<const uint8_t> tx);

    expected<void, int> write(tos::span<const uint8_t> tx);

    spi0(spi_mode::master_t);
    spi0(spi_mode::slave_t);
    ~spi0();

    void isr();

private:
    tos::semaphore spi_block{0};

    tos::span<uint8_t> rx_buf{nullptr};
    tos::span<const uint8_t> tx_buf{nullptr};
};
} // namespace avr

inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::master_t type) {
    return {type};
}

inline avr::spi0 open_impl(tos::devs::spi_t<0> /*tag*/, spi_mode::slave_t type) {
    return {type};
} // namespace avr
} // namespace tos