//
// Created by fatih on 4/15/18.
//

#include <arch/spi.hpp>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <arch/gpio.hpp>
#include <tos/event.hpp>
#include <tos/semaphore.hpp>
#include <common/gpio.hpp>

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
        gp.set_pin_mode(13_pin, pin_mode::out); // SCK
        gp.set_pin_mode(12_pin, pin_mode::in); // MISO
        gp.set_pin_mode(11_pin, pin_mode::out); // MOSI
        gp.set_pin_mode(10_pin, pin_mode::out); // SS
        gp.write(10_pin, true);
        control_reg().init_master();
    }

    void spi0::init_slave() {
        using namespace tos::tos_literals;
        gp.set_pin_mode(13_pin, pin_mode::in); // SCK
        gp.set_pin_mode(12_pin, pin_mode::out); // MISO
        gp.set_pin_mode(11_pin, pin_mode::in); // MOSI
        gp.set_pin_mode(10_pin, pin_mode::in); // SS
        control_reg().init_slave();
    }

    static tos::semaphore spi_block{0};

    static uint8_t *buffer_begin, *buffer_end;

    expected<void, int> spi0::exchange(tos::span<uint8_t> buffer) {
        buffer_begin = buffer.begin();
        buffer_end = buffer.end();
        SPDR = *buffer_begin;
        spi_block.down();
        return {};
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
