//
// Created by fatih on 4/15/18.
//

#include <spi.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <gpio.hpp>
#include <tos/event.hpp>
#include <tos/semaphore.hpp>
#include <drivers/common/gpio.hpp>

namespace {
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
            clk_rate = 0b00;
            SPSR = 0;
        }

        void init_slave() volatile {
            master_slave = 0;
        }
    };
}

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
        using namespace tos::tos_literals;
        gp.set_pin_mode(13_pin, pin_mode_t::out); // SCK
        gp.set_pin_mode(12_pin, pin_mode_t::in); // MISO
        gp.set_pin_mode(11_pin, pin_mode_t::out); // MOSI
        gp.set_pin_mode(10_pin, pin_mode_t::out); // SS
        gp.write(10_pin, true);
        control_reg().init_master();
    }

    void spi0::init_slave() {
        using namespace tos::tos_literals;
        gp.set_pin_mode(13_pin, pin_mode_t::in); // SCK
        gp.set_pin_mode(12_pin, pin_mode_t::out); // MISO
        gp.set_pin_mode(11_pin, pin_mode_t::in); // MOSI
        gp.set_pin_mode(10_pin, pin_mode_t::in); // SS
        control_reg().init_slave();
    }

    static tos::semaphore spi_block{0};

    static uint8_t *buffer_begin, *buffer_end;
    uint8_t spi0::exchange(uint8_t byte) {
        exchange_many(&byte, 1);
        return byte;
    }

    void spi0::exchange_many(uint8_t *buffer, uint16_t len) {
        buffer_begin = buffer;
        buffer_end = buffer + len;
        SPDR = *buffer_begin;
        spi_block.down();
    }

    void spi0::select_slave(pin_t pin) {
        gp.write(pin, false);
    }

    void spi0::deselect_slave(pin_t pin) {
        gp.write(pin, true);
    }

    void spi0::enable() {
        control_reg().enable();
    }

    void spi0::disable() {
        control_reg().disable();
    }
}
}

ISR (SPI_STC_vect) {
    *tos::avr::buffer_begin++ = SPDR;
    if (tos::avr::buffer_begin == tos::avr::buffer_end)
    {
        tos::avr::spi_block.up();
        return;
    }
    SPDR = *tos::avr::buffer_begin;
}
