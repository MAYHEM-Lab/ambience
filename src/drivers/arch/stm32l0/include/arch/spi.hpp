//
// Created by fatih on 5/14/19.
//

#pragma once

#include <common/driver_base.hpp>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/span.hpp>

namespace tos
{
namespace stm32
{
    namespace detail
    {
        struct spi_def
        {
            uint32_t spi;
            rcc_periph_clken clk;
            rcc_periph_rst rst;
        };

        constexpr std::array<spi_def, 1> spis {
            { SPI1, RCC_SPI1, RST_SPI1 }
        };
    }

    class spi : public self_pointing<spi>
    {
    public:
        explicit spi(const detail::spi_def*){
        }

        uint8_t exchange(uint8_t byte);
        void exchange_many(tos::span<uint8_t> buffer);
    };
}
}