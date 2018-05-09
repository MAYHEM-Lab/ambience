//
// Created by fatih on 4/17/18.
//

#pragma once

#include <stdint.h>
#include <common/gpio.hpp>
#include <tos/devices.hpp>
#include <common/sd/sd_info.hpp>

namespace tos
{
    class spi_sd_card
    {
    public:
        explicit spi_sd_card(pin_t spi_pin);

        bool init();
        void read(void* to, uint32_t blk, uint16_t len, uint16_t offset = 0);
        void write(const void* from, uint32_t blk, uint16_t len, uint16_t offset = 0);

        csd_t read_csd();

    private:

        pin_t m_spi_pin;
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
        } else if (csd.v2.csd_ver == 1) {
            uint32_t c_size = ((uint32_t)csd.v2.c_size_high << 16)
                              | (csd.v2.c_size_mid << 8) | csd.v2.c_size_low;
            return (c_size + 1) << 10;
        }
        return 0;
    }

    constexpr inline uint16_t get_blk_size(const csd_t&)
    {
        return 512;
    }

    namespace devs
    {
        using sd_t = dev<struct _sd_t, 0>;
        static constexpr sd_t sd{};
    }

    inline spi_sd_card open_impl(devs::sd_t, pin_t pin)
    {
        return spi_sd_card(pin);
    }
}