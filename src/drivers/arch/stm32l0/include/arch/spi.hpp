//
// Created by fatih on 5/14/19.
///

#pragma once

#include <arch/gpio.hpp>
#include <common/driver_base.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/spi.h>
#include <tos/expected.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/semaphore.hpp>
#include <tos/span.hpp>

extern void spi3_isr();

namespace tos {
namespace stm32 {
namespace detail {
struct spi_def {
    uint32_t spi;
    rcc_periph_clken clk;
    rcc_periph_rst rst;
    uint8_t irq;
};

constexpr std::array<spi_def, 3> spis{
    spi_def{SPI1, RCC_SPI1, RST_SPI1, NVIC_SPI1_IRQ},
    spi_def{SPI2, RCC_SPI2, RST_SPI2, NVIC_SPI2_IRQ},
#ifdef SPI3_BASE
    spi_def{SPI3, RCC_SPI3, RST_SPI3, NVIC_SPI3_IRQ},
#endif
};
} // namespace detail

enum class spi_errors
{
    bad_mode
};

class spi
    : public self_pointing<spi>
    , public tracked_driver<spi, detail::spis.size()> {
public:
    using gpio_type = stm32::gpio;

    explicit spi(const detail::spi_def& def,
                 gpio::pin_type sck,
                 std::optional<gpio::pin_type> miso,
                 std::optional<gpio::pin_type> mosi)
        : tracked_driver{std::distance(&detail::spis[0], &def)}
        , m_def{&def} {

        rcc_periph_clock_enable(sck.port->rcc);
        gpio_mode_setup(sck.port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, sck.pin);
        gpio_set_af(sck.port->which, GPIO_AF6, sck.pin);

        if (miso)
        {
            rcc_periph_clock_enable(miso->port->rcc);
            gpio_mode_setup(miso->port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, miso->pin);
            gpio_set_af(miso->port->which, GPIO_AF6, miso->pin);
        }

        if (mosi)
        {
            rcc_periph_clock_enable(mosi->port->rcc);
            gpio_mode_setup(mosi->port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, mosi->pin);
            gpio_set_af(mosi->port->which, GPIO_AF6, mosi->pin);
        }

        rcc_periph_reset_pulse(m_def->rst);
        rcc_periph_clock_enable(m_def->clk);

        SPI_CR1(m_def->spi) = 0;
        SPI_CR1(m_def->spi) =
            SPI_CR1_BAUDRATE_FPCLK_DIV_8 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;

        // phase 0, polarity 0
        // not really needed
        //        SPI_CR1(m_def->spi) &= ~SPI_CR1_CPHA;
        //        SPI_CR1(m_def->spi) &= ~SPI_CR1_CPOL;

        SPI_CR2(m_def->spi) = 0;
#ifdef STM32L4
        SPI_CR2(m_def->spi) |= SPI_CR2_FRXTH;
#endif

        SPI_CR1(m_def->spi) |= SPI_CR1_SPE;

        nvic_enable_irq(m_def->irq);
    }

    expected<void, spi_errors> write(span<const uint8_t> buffer) {
        if (in_16_bit_mode()) {
            return unexpected(spi_errors::bad_mode);
        }

        m_write = buffer;
        m_read = tos::empty_span<uint8_t>();

        enable_tx_isr();

        m_done.down();
        while (SPI_SR(m_def->spi) & SPI_SR_BSY)
            ;
        return {};
    }

    expected<void, spi_errors> exchange(span<uint8_t> rx, span<const uint8_t> tx) {
        if (in_16_bit_mode()) {
            return unexpected(spi_errors::bad_mode);
        }

        m_write = tx;
        m_read = rx;

        // clear rx fifo
        for (int i = 0; i < 4; ++i) {
            volatile uint8_t c = SPI_DR8(m_def->spi);
        }

        enable_rx_tx_isr();

        m_done.down();
        while (SPI_SR(m_def->spi) & SPI_SR_BSY)
            ;

        return {};
    }

    void set_8_bit_mode() {
#ifdef STM32L0
        SPI_CR1(m_def->spi) &= ~SPI_CR1_DFF_16BIT;
        SPI_CR1(m_def->spi) |= SPI_CR1_DFF_8BIT;
#endif
#ifdef STM32L4
        SPI_CR2(m_def->spi) &= ~SPI_CR2_DS_16BIT;
        SPI_CR2(m_def->spi) |= SPI_CR2_DS_8BIT;
#endif
    }

    void set_16_bit_mode() {
#ifdef STM32L0
        SPI_CR1(m_def->spi) &= ~SPI_CR1_DFF_8BIT;
        SPI_CR1(m_def->spi) |= SPI_CR1_DFF_16BIT;
#endif
#ifdef STM32L4
        SPI_CR2(m_def->spi) &= ~SPI_CR2_DS_8BIT;
        SPI_CR2(m_def->spi) |= SPI_CR2_DS_16BIT;
#endif
    }

    bool in_16_bit_mode() const {
#ifdef STM32L0
        return (SPI_CR1(m_def->spi) & SPI_CR1_DFF_16BIT) == SPI_CR1_DFF_16BIT;
#endif
#ifdef STM32L4
        return (SPI_CR2(m_def->spi) & SPI_CR2_DS_16BIT) == SPI_CR2_DS_16BIT;
#endif
    }

private:
    void enable_rx_tx_isr() {
        SPI_CR2(m_def->spi) |= SPI_CR2_RXNEIE | SPI_CR2_TXEIE;
    }

    void enable_rx_isr() {
        SPI_CR2(m_def->spi) |= SPI_CR2_RXNEIE;
    }

    void disable_rx_isr() {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_RXNEIE;
    }

    bool rx_isr_enabled() const {
        return SPI_CR2(m_def->spi) & SPI_CR2_RXNEIE;
    }

    void enable_tx_isr() {
        SPI_CR2(m_def->spi) |= SPI_CR2_TXEIE;
    }

    void disable_tx_isr() {
        SPI_CR2(m_def->spi) &= ~SPI_CR2_TXEIE;
    }

    bool tx_isr_enabled() const {
        return SPI_CR2(m_def->spi) & SPI_CR2_TXEIE;
    }

    void isr() {
        if (tx_isr_enabled() && SPI_SR(m_def->spi) & SPI_SR_TXE) {
            if (m_write.empty()) {
                disable_tx_isr();
            } else {
#ifdef STM32L4
                SPI_DR8(m_def->spi) = m_write[0];
#elif defined(STM32L0)
                SPI_DR(m_def->spi) = m_write[0];
#endif
                m_write = m_write.slice(1);
                return;
            }
        }

        if (rx_isr_enabled() && SPI_SR(m_def->spi) & SPI_SR_RXNE) {
            Expects(!m_read.empty());
#ifdef STM32L4
            m_read[0] = SPI_DR8(m_def->spi);
#elif defined(STM32L0)
            m_read[0] = SPI_DR(m_def->spi);
#endif

            m_read = m_read.slice(1);

            if (m_read.empty()) {
                disable_rx_isr();
            }
        }

        if (m_read.empty() && m_write.empty()) {
            m_done.up_isr();
        }
    }

    friend void ::spi1_isr();
    friend void ::spi2_isr();
    friend void ::spi3_isr();

    tos::span<const uint8_t> m_write{nullptr};
    tos::span<uint8_t> m_read{nullptr};
    tos::semaphore m_done{0};
    const detail::spi_def* m_def;
};
} // namespace stm32
} // namespace tos
