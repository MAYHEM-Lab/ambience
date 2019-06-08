//
// Created by fatih on 4/17/18.
//

#pragma once

#include <stdint.h>
#include <common/gpio.hpp>
#include <tos/devices.hpp>
#include <common/sd/sd_info.hpp>
#include <tos/delay.hpp>
#include <common/spi.hpp>

namespace tos
{
    template <class SpiT>
    class spi_sd_card
    {
    public:
        using gpio_t = typename SpiT::gpio_type;

        using pin_t = typename SpiT::gpio_type::pin_type;

        explicit spi_sd_card(SpiT& spi, gpio_t& g, pin_t cs) : m_spi{&spi}, m_gpio{&g}, m_cs{cs} {}

        bool init();
        void read(void* to, uint32_t blk, uint16_t len, uint16_t offset = 0);

        csd_t read_csd();

    private:
        spi_transaction<SpiT> exec_cmd(uint8_t cmd, uint32_t arg, uint8_t crc = 0xFF)
        {
            spi_transaction<SpiT> tr(*m_spi, *m_gpio, m_cs);

            tr->exchange(cmd);
            tr->exchange(arg >> 24); // MSB first
            tr->exchange(arg >> 16);
            tr->exchange(arg >> 8);
            tr->exchange(arg);
            tr->exchange(crc);

            return tr;
        }

        uint8_t read_8(spi_transaction<SpiT>&& tr)
        {
            uint8_t ret = 0xFF;

            for (int i = 0; i < 8; ++i)
            {
                auto x = tr->exchange(0xFF);
                if (x != 0xFF) ret = x;
            }

            return ret;
        }

        typename SpiT::gpio_type* m_gpio;
        SpiT* m_spi;
        pin_t m_cs;
    };

    constexpr inline uint32_t get_blk_count(const csd_t& csd)
    {
        if (csd.v1.csd_ver == 0) {
            uint8_t read_bl_len = csd.v1.read_bl_len;
            uint16_t c_size = (csd.v1.c_size_high << 10)
                              | (csd.v1.c_size_mid << 2) | csd.v1.c_size_low;
            uint8_t c_size_mult = (csd.v1.c_size_mult_high << 1)
                                  | csd.v1.c_size_mult_low;
            return (uint32_t)(c_size + 1) << (c_size_mult + read_bl_len - 7);
        }

        if (csd.v2.csd_ver == 1) {
            uint32_t c_size = ((uint32_t)csd.v2.c_size_high << 16)
                              | (csd.v2.c_size_mid << 8) | csd.v2.c_size_low;
            return (c_size + 1) << 10;
        }

        return 0;
    }

    constexpr inline uint16_t get_blk_size(const csd_t& /*csd*/)
    {
        return 512;
    }

    namespace devs
    {
        using sd_t = dev<struct _sd_t, 0>;
        static constexpr sd_t sd{};
    } // namespace devs

    template <class SpiT>
    inline spi_sd_card<SpiT> open_impl(devs::sd_t /*sd*/, SpiT& spi, typename SpiT::gpio_type& g, typename SpiT::gpio_type::pin_type pin)
    {
        return spi_sd_card<SpiT>(spi, g, pin);
    }
}

// impl

namespace tos
{
    template <class SpiT>
    bool spi_sd_card<SpiT>::init() {
        for (int i = 0; i < 10; ++i)
        {
            m_spi->exchange(0xFF);
        }

        using namespace std::chrono_literals;
        int16_t i;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x40 + 0, 0x00000000, 0x95)) == 0x01) break;
            tos::delay_ms(100ms);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x40 + 8, 0x000001AA, 0x87)) == 0xAA) break;
            tos::delay_ms(100ms);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x40 + 55, 0x0)) != 0xFF
                && read_8(exec_cmd(0x40 + 41, 0x40000000)) == 0x00)
            {
                break;
            }
            tos::delay_ms(100ms);
        }
        if (i == 10) return false;

        read_8(exec_cmd(0x40 + 16, 0x00000200, 0xFF));

        return true;
    }

    template <class SpiT>
    void spi_sd_card<SpiT>::read(void *to, uint32_t blk, uint16_t len, uint16_t offset) {

        auto tr = exec_cmd(0x40 + 17, blk, 0xFF);

        while (tr->exchange(0xFF) != 0x00);
        while (tr->exchange(0xFF) != 0xFE);

        uint16_t i;
        for (i = 0; i < offset; ++i)
        {
            tr->exchange(0xFF);
        }

        auto target = reinterpret_cast<uint8_t*>(to);
        tr->exchange_many({target, len});
        i += len;

        for (; i < 512 + 3; ++i) // is the 3 fixed?
        {
            tr->exchange(0xFF);
        }
    }

    template <class SpiT>
    csd_t spi_sd_card<SpiT>::read_csd() {
        auto tr = exec_cmd(0x40 + 9, 0, 0xFF);

        while (tr->exchange(0xFF) != 0x00);
        while (tr->exchange(0xFF) != 0xFE);

        csd_t res{};
        auto res_ptr = reinterpret_cast<uint8_t*>(&res);
        for (int i = 0; i < 16; ++i)
        {
            res_ptr[i] = tr->exchange(0xFF);
        }

        tr->exchange(0xFF);
        tr->exchange(0xFF);
        tr->exchange(0xFF);
        return res;
    }
} // namespace tos