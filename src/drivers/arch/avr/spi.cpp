//
// Created by fatih on 4/15/18.
//

#include <spi.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <gpio.hpp>
#include <tos/semaphore.hpp>

struct spi_ctrl
{
    uint8_t clk_rate : 2;
    uint8_t clk_phase : 1;
    uint8_t clk_polarity : 1;
    uint8_t master_slave : 1;
    uint8_t data_order : 1;
    uint8_t spi_enable : 1;
    uint8_t spi_int_en : 1;

    void enable() volatile
    {
        PRR &= ~(1 << PRSPI);
        spi_enable = true;
        spi_int_en = true;
    }

    void disable() volatile
    {
        PRR |= (1 << PRSPI);
        spi_enable = false;
        spi_int_en = false;
    }

    void init_master() volatile {
        master_slave = 1;
        clk_rate = 0b01;
    }

    void init_slave() volatile {
        master_slave = 0;
    }
};

static_assert(sizeof(spi_ctrl) == 1, "");

static volatile spi_ctrl& control_reg()
{
    return *reinterpret_cast<volatile spi_ctrl*>(&SPCR);
}

namespace tos
{
namespace avr
{
    static gpio gp;
    void spi0::init_master() {
        gp.set_pin_mode(ports::B, 5, gpio::pin_mode_t::out); // SCK
        gp.set_pin_mode(ports::B, 4, gpio::pin_mode_t::in); // MISO
        gp.set_pin_mode(ports::B, 3, gpio::pin_mode_t::out); // MOSI
        gp.set_pin_mode(ports::B, 2, gpio::pin_mode_t::out); // SS
        gp.write(ports::B, 2, true);
        control_reg().init_master();
    }

    void spi0::init_slave() {
        gp.set_pin_mode(ports::B, 5, gpio::pin_mode_t::in); // SCK
        gp.set_pin_mode(ports::B, 4, gpio::pin_mode_t::out); // MISO
        gp.set_pin_mode(ports::B, 3, gpio::pin_mode_t::in); // MOSI
        gp.set_pin_mode(ports::B, 2, gpio::pin_mode_t::in); // SS
        control_reg().init_slave();
    }

    void spi0::enable() {
        control_reg().enable();
    }

    void spi0::disable() {
        control_reg().disable();
    }

    static ft::semaphore spi_block {0};

    void spi_write_byte(uint8_t d) {
        gp.write(ports::B, 2, false);
        SPDR = d;
        spi_block.down();
        //while(!(SPSR & (1<<SPIF)));
        gp.write(ports::B, 2, true);
    }

    uint8_t spi_read_byte() {
        //while(!(SPSR & (1<<SPIF)));
        spi_block.down();
        return SPDR;
    }
}
}

ISR (SPI_STC_vect) {
    tos::avr::spi_block.up();
}
