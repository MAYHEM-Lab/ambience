//
// Created by fatih on 4/17/18.
//

#pragma once

#include <stdint.h>
#include <common/gpio.hpp>
#include <tos/devices.hpp>

namespace tos
{
    class spi_sd_card
    {
    public:
        explicit spi_sd_card(pin_id spi_pin);

        bool init();
        void read(void* to, uint32_t blk, uint16_t len, uint16_t offset = 0);
        void write(const void* from, uint32_t blk, uint16_t len, uint16_t offset = 0);

    private:
        pin_id m_spi_pin;
    };

    namespace devs
    {
        using sd_t = dev<struct _sd_t, 0>;
        static constexpr sd_t sd{};
    }

    inline spi_sd_card open_impl(devs::sd_t, pin_id pin)
    {
        return spi_sd_card(pin);
    }
}