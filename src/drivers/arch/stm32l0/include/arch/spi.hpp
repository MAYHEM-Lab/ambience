//
// Created by fatih on 5/14/19.
//

#pragma once

#include <common/driver_base.hpp>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/span.hpp>

inline char buf[4000];
inline int idx = 0;

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
        explicit spi(const detail::spi_def& def) : m_def{&def} {
            rcc_periph_reset_pulse(m_def->rst);
            rcc_periph_clock_enable(m_def->clk);

            SPI_CR1(m_def->spi) = 0;
            SPI_CR1(m_def->spi) |= SPI_CR1_BAUDRATE_FPCLK_DIV_4 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
            SPI_CR1(m_def->spi) |= SPI_CR1_SPE;
        }

        uint8_t exchange(uint8_t byte)
        {
            buf[idx++] = byte;
            return spi_xfer(m_def->spi, byte);
        }

        void exchange_many(tos::span<uint8_t> buffer)
        {
            for (uint8_t& c : buffer)
            {
                c = exchange(c);
            }
        }

        void write(uint8_t c)
        {
            buf[idx++] = c;
            SPI_DR(m_def->spi) = c;
            while(!(SPI_SR(m_def->spi) & SPI_SR_TXE));
            while(SPI_SR(m_def->spi) & SPI_SR_BSY);
        }

        void write(tos::span<const uint8_t> buffer)
        {
            for (uint8_t c : buffer)
            {
                write(c);
            }
        }

    private:

        const detail::spi_def* m_def;
    };
}
}