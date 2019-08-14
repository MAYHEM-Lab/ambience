//
// Created by fatih on 8/11/19.
//

#pragma once

#include <arch/gpio.hpp>
#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/semaphore.hpp>
#include <tos/utility.hpp>

namespace tos {
namespace stm32 {
namespace detail {
struct i2c_def {
    uint32_t i2c;
    rcc_periph_clken rcc;
    rcc_periph_rst rst;
    uint8_t ev_irq;
    uint8_t er_irq;
};

constexpr i2c_def i2cs[] = {
    {I2C1, RCC_I2C1, RST_I2C1, NVIC_I2C1_EV_IRQ, NVIC_I2C1_ER_IRQ},
    {I2C2, RCC_I2C2, RST_I2C2, NVIC_I2C2_EV_IRQ, NVIC_I2C2_ER_IRQ},
};
} // namespace detail

class i2c
    : public self_pointing<i2c>
    , public tracked_driver<i2c, std::size(detail::i2cs)> {
public:
    explicit i2c(const detail::i2c_def& dev,
                 gpio::pin_type clk,
                 gpio::pin_type data) noexcept;

    twi_tx_res transmit(twi_addr_t to, tos::span<const char> data);

    twi_rx_res receive(twi_addr_t from, span<char> buffer);

    void isr();
    void err_isr();

private:
    void enable_isrs() {
        I2C_CR1(m_dev->i2c) |=
            I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE | I2C_CR1_STOPIE;
    }

    void disable_tx_isr() { I2C_CR1(m_dev->i2c) &= ~I2C_CR1_TXIE; }
    void disable_rx_isr() { I2C_CR1(m_dev->i2c) &= ~I2C_CR1_RXIE; }
    void disable_stop_isr() { I2C_CR1(m_dev->i2c) &= ~I2C_CR1_STOPIE; }
    void disable_nack_isr() { I2C_CR1(m_dev->i2c) &= ~I2C_CR1_NACKIE; }

    bool busy() const { return I2C_ISR(m_dev->i2c) & I2C_ISR_BUSY; }

    void clear_flags() {
        I2C_ICR(m_dev->i2c) = 0xFFFF;
        I2C_CR2(m_dev->i2c) = 0;
        m_error = false;
    }

    const detail::i2c_def* m_dev;

    bool m_error = false;
    tos::semaphore m_done{0};
    tos::span<const char> m_write{nullptr};
    tos::span<char> m_read{nullptr};
};
} // namespace stm32
} // namespace tos

// impl

namespace tos {
namespace stm32 {
inline void i2c::err_isr() {
    m_error = true;
    disable_stop_isr();
    disable_rx_isr();
    disable_rx_isr();
    m_done.up_isr();
}

inline void i2c::isr() {
    auto isr_reg = I2C_ISR(m_dev->i2c);

    if ((isr_reg & I2C_ISR_NACKF) == I2C_ISR_NACKF) {
        disable_nack_isr();
        err_isr();
        I2C_ICR(m_dev->i2c) |= I2C_ICR_NACKCF;
        return;
    }

    // transmit buffer empty
    if ((isr_reg & I2C_ISR_TXIS) == I2C_ISR_TXIS) {
        I2C_TXDR(m_dev->i2c) = m_write.front();
        m_write = m_write.slice(1, m_write.size() - 1);
        if (m_write.empty()) {
            disable_tx_isr();
        }
    }

    // receive buffer not empty
    if ((isr_reg & I2C_ISR_RXNE) == I2C_ISR_RXNE) {
        m_read.front() = I2C_RXDR(m_dev->i2c);
        m_read = m_read.slice(1, m_read.size() - 1);
        if (m_read.empty()) {
            disable_rx_isr();
        }
    }

    // transfer complete
    if ((isr_reg & I2C_ISR_STOPF) == I2C_ISR_STOPF) {
        disable_stop_isr();
        I2C_ICR(m_dev->i2c) |= I2C_ICR_STOPCF;
        m_done.up_isr();
    }
}

inline i2c::i2c(const detail::i2c_def& dev,
                gpio::pin_type clk,
                gpio::pin_type data) noexcept
    : tracked_driver(std::distance(&detail::i2cs[0], &dev))
    , m_dev{&dev} {
    rcc_periph_clock_enable(m_dev->rcc);
    rcc_periph_reset_pulse(m_dev->rst);

    rcc_periph_clock_enable(data.port->rcc);
    rcc_periph_clock_enable(clk.port->rcc);

    gpio_mode_setup(clk.port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, clk.pin);
    gpio_set_output_options(clk.port->which, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, clk.pin);
    gpio_set_af(clk.port->which, GPIO_AF4, clk.pin);

    gpio_mode_setup(data.port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, data.pin);
    gpio_set_output_options(data.port->which, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ, data.pin);
    gpio_set_af(data.port->which, GPIO_AF4, data.pin);

    // disable the peripheral
    I2C_CR1(m_dev->i2c) = 0;

    i2c_set_speed(m_dev->i2c, i2c_speeds::i2c_speed_sm_100k, rcc_apb1_frequency / 1e6);

    I2C_CR1(m_dev->i2c) |= I2C_CR1_PE;

    nvic_enable_irq(m_dev->ev_irq);
}

inline twi_tx_res i2c::transmit(twi_addr_t to, tos::span<const char> data) {
    while (busy())
        ;

    m_write = data;

    clear_flags();

    I2C_CR2(m_dev->i2c) = (I2C_CR2(m_dev->i2c) & ~I2C_CR2_SADD_7BIT_MASK) |
                          ((to.addr & 0x7F) << I2C_CR2_SADD_7BIT_SHIFT);

    i2c_set_bytes_to_transfer(m_dev->i2c, data.size());

    I2C_CR2(m_dev->i2c) &= ~I2C_CR2_RD_WRN;
    I2C_CR2(m_dev->i2c) |= I2C_CR2_AUTOEND;

    enable_isrs();

    I2C_CR2(m_dev->i2c) |= I2C_CR2_START;

    m_done.down();

    if (m_error) {
        return twi_tx_res::addr_nack;
    }

    while (busy())
        ;

    return twi_tx_res::ok;
}

inline twi_rx_res i2c::receive(twi_addr_t from, span<char> buffer) {
    m_read = buffer;

    clear_flags();

    I2C_CR2(m_dev->i2c) = (I2C_CR2(m_dev->i2c) & ~I2C_CR2_SADD_7BIT_MASK) |
                          ((from.addr & 0x7F) << I2C_CR2_SADD_7BIT_SHIFT);

    i2c_set_bytes_to_transfer(m_dev->i2c, buffer.size());

    I2C_CR2(m_dev->i2c) |= I2C_CR2_RD_WRN;
    I2C_CR2(m_dev->i2c) |= I2C_CR2_AUTOEND;

    enable_isrs();

    I2C_CR2(m_dev->i2c) |= I2C_CR2_START;

    m_done.down();

    if (m_error) {
        return twi_rx_res::addr_nack;
    }

    while (busy())
        ;

    return twi_rx_res::ok;
}
} // namespace stm32
} // namespace tos