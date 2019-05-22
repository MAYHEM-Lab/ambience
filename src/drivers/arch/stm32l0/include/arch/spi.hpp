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
        explicit spi(const detail::spi_def& def) : m_def{&def} {
            rcc_periph_reset_pulse(m_def->rst);
            rcc_periph_clock_enable(m_def->clk);

            spi_set_master_mode(m_def->spi);
            spi_set_baudrate_prescaler(m_def->spi, SPI_CR1_BR_FPCLK_DIV_64);
            spi_set_clock_polarity_0(m_def->spi);
            spi_set_clock_phase_0(m_def->spi);
            spi_set_full_duplex_mode(m_def->spi);
            spi_set_unidirectional_mode(m_def->spi); /* bidirectional but in 3-wire */
            spi_enable_software_slave_management(m_def->spi);
            spi_send_msb_first(m_def->spi);
            spi_set_nss_high(m_def->spi);
            //spi_
            //spi_enable_ss_output(SPI1);
            //spi_fifo_reception_threshold_8bit(SPI1);
            SPI_I2SCFGR(m_def->spi) &= ~SPI_I2SCFGR_I2SMOD;
            spi_enable(m_def->spi);
        }

        uint8_t exchange(uint8_t byte)
        {
            return spi_xfer(m_def->spi, byte);
        }

        void exchange_many(tos::span<uint8_t> buffer)
        {
            for (uint8_t& c : buffer)
            {
                c = exchange(c);
            }
        }

        void write(tos::span<const uint8_t> buffer)
        {
            for (uint8_t c : buffer)
            {
                exchange(c);
            }
        }

    private:

        const detail::spi_def* m_def;
    };
}
}