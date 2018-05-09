//
// Created by fatih on 4/17/18.
//

#include <arch/avr/gpio.hpp>
#include <common/sd/spi_sd.hpp>
#include <util/delay.h>
#include <arch/avr/spi.hpp>

namespace tos
{
    namespace arch = avr;
}

namespace tos
{
    static spi_transaction<avr::spi0> exec_cmd(pin_t p, uint8_t cmd, uint32_t arg, uint8_t crc = 0xFF)
    {
        spi_transaction<avr::spi0> tr{p};

        tr.exchange(cmd);
        tr.exchange(arg >> 24); // MSB first
        tr.exchange(arg >> 16);
        tr.exchange(arg >> 8);
        tr.exchange(arg);
        tr.exchange(crc);

        return tr;
    }

    static uint8_t read_8(spi_transaction<avr::spi0>&& tr)
    {
        uint8_t ret = 0xFF;

        for (int i = 0; i < 8; ++i)
        {
            auto x = tr.exchange(0xFF);
            if (x != 0xFF) ret = x;
        }

        return ret;
    }

    spi_sd_card::spi_sd_card(pin_t spi_pin) : m_spi_pin{spi_pin} {
    }

    bool spi_sd_card::init() {
        for (int i = 0; i < 10; ++i)
        {
            arch::spi0::exchange(0xFF);
        }

        int16_t i;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(m_spi_pin, 0x40 + 0, 0x00000000, 0x95)) == 0x01) break;
            _delay_ms(100);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(m_spi_pin, 0x40 + 8, 0x000001AA, 0x87)) == 0xAA) break;
            _delay_ms(100);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(m_spi_pin, 0x40 + 55, 0x0)) != 0xFF && read_8(exec_cmd(m_spi_pin,0x40 + 41, 0x40000000)) == 0x00) break;
            _delay_ms(100);
        }
        if (i == 10) return false;

        read_8(exec_cmd(m_spi_pin, 0x40 + 16, 0x00000200, 0xFF));

        return true;
    }

    void spi_sd_card::read(void *to, uint32_t blk, uint16_t len, uint16_t offset) {

        auto tr = exec_cmd(m_spi_pin, 0x40 + 17, blk, 0xFF);

        while (tr.exchange(0xFF) != 0x00);
        while (tr.exchange(0xFF) != 0xFE);

        uint16_t i;
        for (i = 0; i < offset; ++i)
        {
            tr.exchange(0xFF);
        }

        auto target = reinterpret_cast<uint8_t*>(to);
        tr.exchange_many(target, len);
        i += len;

        for (; i < 512 + 3; ++i) // is the 3 fixed?
        {
            tr.exchange(0xFF);
        }
    }

    csd_t spi_sd_card::read_csd() {
        auto tr = exec_cmd(m_spi_pin,0x40 + 9, 0, 0xFF);

        while (tr.exchange(0xFF) != 0x00);
        while (tr.exchange(0xFF) != 0xFE);

        csd_t res{};
        auto res_ptr = reinterpret_cast<uint8_t*>(&res);
        for (int i = 0; i < 16; ++i)
        {
            res_ptr[i] = tr.exchange(0xFF);
        }

        tr.exchange(0xFF);
        tr.exchange(0xFF);
        tr.exchange(0xFF);
        return res;
    }
}