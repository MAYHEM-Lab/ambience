//
// Created by fatih on 5/14/19.
///

#pragma once

#include <common/driver_base.hpp>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/span.hpp>
#include <tos/semaphore.hpp>
#include <arch/gpio.hpp>
#include <tos/fixed_fifo.hpp>

extern tos::fixed_fifo<char, 128> pr;

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
    uint8_t irq;
};

constexpr std::array<spi_def, 3> spis {
    spi_def{ SPI1, RCC_SPI1, RST_SPI1, NVIC_SPI1_IRQ },
    spi_def{ SPI2, RCC_SPI2, RST_SPI2, NVIC_SPI2_IRQ },
    spi_def{ SPI3, RCC_SPI3, RST_SPI3, NVIC_SPI3_IRQ },
};
}

class spi :
        public self_pointing<spi>,
        public tracked_driver<spi, 3>
{
public:
    using gpio_type = stm32::gpio;

    explicit spi(const detail::spi_def& def)
        : tracked_driver{std::distance(&detail::spis[0], &def)}
        , m_def{&def}
    {
        rcc_periph_reset_pulse(m_def->rst);
        rcc_periph_clock_enable(m_def->clk);

        SPI_CR1(m_def->spi) = 0;
        SPI_CR1(m_def->spi) = SPI_CR1_BAUDRATE_FPCLK_DIV_8 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;

        // phase 0, polarity 0
        // not really needed
//        SPI_CR1(m_def->spi) &= ~SPI_CR1_CPHA;
//        SPI_CR1(m_def->spi) &= ~SPI_CR1_CPOL;

        SPI_CR2(m_def->spi) = 0;
        SPI_CR2(m_def->spi) |= SPI_CR2_FRXTH;

        SPI_CR1(m_def->spi) |= SPI_CR1_SPE;

        nvic_enable_irq(m_def->irq);
    }

    uint16_t exchange16(uint16_t data)
    {
        Expects(in_16_bit_mode());
        m_write = {&data, 1};
        uint16_t out;
        m_read = {&out, 1};
        enable_rx_isr();
        enable_tx_isr();
        m_done.down();
        while(SPI_SR(m_def->spi) & SPI_SR_BSY);
        return out;
    }

    uint8_t exchange8(uint8_t data) {
        Expects(!in_16_bit_mode());
        uint16_t d = data;
        m_write = {&d, 1};
        uint16_t out;
        m_read = {&out, 1};
        enable_rx_isr();
        enable_tx_isr();
        m_done.down();
        while(SPI_SR(m_def->spi) & SPI_SR_BSY);
        return out;
    }

    void set_8_bit_mode() {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_DS_16BIT;
        SPI_CR2(m_def->spi) |= SPI_CR2_DS_8BIT;
    }

    void set_16_bit_mode() {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_DS_8BIT;
        SPI_CR2(m_def->spi) |= SPI_CR2_DS_16BIT;
    }

    bool in_16_bit_mode() const {
        return SPI_CR2(m_def->spi) & SPI_CR2_DS_16BIT;
    }

private:
    void enable_rx_isr()
    {
        SPI_CR2(m_def->spi) |= SPI_CR2_RXNEIE;
    }

    void disable_rx_isr()
    {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_RXNEIE;
    }

    bool rx_isr_enabled() const {
        return SPI_CR2(m_def->spi) & SPI_CR2_RXNEIE;
    }

    void enable_tx_isr()
    {
        SPI_CR2(m_def->spi) |= SPI_CR2_TXEIE;
    }

    void disable_tx_isr()
    {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_TXEIE;
    }

    bool tx_isr_enabled() const {
        return SPI_CR2(m_def->spi) & SPI_CR2_TXEIE;
    }

    void isr()
    {
        if (tx_isr_enabled() && SPI_SR(m_def->spi) & SPI_SR_TXE)
        {
            if (m_write.empty())
            {
                disable_tx_isr();
            }
            else
            {
                SPI_DR8(m_def->spi) = m_write[0];
                m_write = m_write.slice(1);
                return;
            }
        }

        if (rx_isr_enabled() && SPI_SR(m_def->spi) & SPI_SR_RXNE)
        {
            Expects(!m_read.empty());
            m_read[0] = SPI_DR8(m_def->spi);
            //pr.push(m_read[0]);
            m_read = m_read.slice(1);

            if (m_read.empty())
            {
                disable_rx_isr();
            }
        }

        if (m_read.empty() && m_write.empty())
        {
            m_done.up_isr();
        }
    }

    friend void ::spi1_isr();
    friend void ::spi2_isr();
    friend void ::spi3_isr();

    tos::span<const uint16_t> m_write{nullptr};
    tos::span<uint16_t> m_read{nullptr};
    tos::semaphore m_done{0};
    const detail::spi_def* m_def;
};
}
}
