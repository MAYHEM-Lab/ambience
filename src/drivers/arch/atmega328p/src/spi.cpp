//
// Created by fatih on 4/15/18.
//

#include "common/i2c.hpp"
#include "tos/debug/debug.hpp"

#include <arch/gpio.hpp>
#include <arch/spi.hpp>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <common/gpio.hpp>
#include <cstdint>
#include <tos/event.hpp>
#include <tos/semaphore.hpp>

namespace {
struct spi_ctrl {
    uint8_t clk_rate : 2;
    uint8_t clk_phase : 1;
    uint8_t clk_polarity : 1;
    uint8_t master_slave : 1;
    uint8_t data_order : 1;
    uint8_t spi_enable : 1;
    uint8_t spi_int_en : 1;

    void enable() volatile {
        PRR &= ~(1 << PRSPI);
        spi_enable = true;
        spi_int_en = true;
    }

    void disable() volatile {
        PRR |= (1 << PRSPI);
        spi_enable = false;
        spi_int_en = false;
    }

    void init_master() volatile {
        master_slave = 1;
        clk_rate = 0b00;
        SPSR = 0;
    }

    void init_slave() volatile {
        master_slave = 0;
    }
};
} // namespace

static_assert(sizeof(spi_ctrl) == 1, "");

static volatile spi_ctrl& control_reg() {
    return *reinterpret_cast<volatile spi_ctrl*>(&SPCR);
}

namespace tos {
namespace avr {
static gpio gp;
void spi0::init_master() {
    using namespace tos::tos_literals;
    gp.set_pin_mode(13_pin, pin_mode::out); // SCK
    gp.set_pin_mode(12_pin, pin_mode::in);  // MISO
    gp.set_pin_mode(11_pin, pin_mode::out); // MOSI
    gp.set_pin_mode(10_pin, pin_mode::out); // SS
    gp.write(10_pin, true);
    control_reg().init_master();
}

void spi0::init_slave() {
    using namespace tos::tos_literals;
    gp.set_pin_mode(13_pin, pin_mode::in);  // SCK
    gp.set_pin_mode(12_pin, pin_mode::out); // MISO
    gp.set_pin_mode(11_pin, pin_mode::in);  // MOSI
    gp.set_pin_mode(10_pin, pin_mode::in);  // SS
    control_reg().init_slave();
}

expected<void, int> spi0::exchange(tos::span<uint8_t> rx, tos::span<const uint8_t> tx) {
    tx_buf = tx;
    rx_buf = rx;
    SPDR = tx_buf.front();
    spi_block.down();
    return {};
}

expected<void, int> spi0::write(tos::span<const uint8_t> tx) {
    tx_buf = tx;
    SPDR = tx_buf.front();
    spi_block.down();
    return {};
}

void spi0::isr() {
    tx_buf = tx_buf.slice(1);

    uint8_t read = SPDR;
    if (!rx_buf.empty()) {
        rx_buf.front() = SPDR;
        rx_buf = rx_buf.slice(1);
    } else {
        tos::debug::do_not_optimize(&read);
    }

    if (tx_buf.empty()) {
        spi_block.up_isr();
        return;
    }

    SPDR = tx_buf.front();
}

spi0::spi0(spi_mode::slave_t)
    : tracked_driver(0) {
    control_reg().enable();
    init_slave();
}

spi0::spi0(spi_mode::master_t)
    : tracked_driver(0) {
    control_reg().enable();
    init_master();
}

spi0::~spi0() {
    control_reg().disable();
}
} // namespace avr
} // namespace tos

ISR(SPI_STC_vect) {
    auto& instance = *tos::avr::spi0::get(0);
    instance.isr();
}
