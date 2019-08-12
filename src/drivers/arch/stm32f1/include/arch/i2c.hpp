//
// Created by fatih on 1/18/19.
//

#pragma once

#include <arch/gpio.hpp>
#include <common/driver_base.hpp>
#include <common/i2c.hpp>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <tos/utility.hpp>

namespace tos {
namespace stm32 {
namespace detail {
struct i2c_def {
    uint32_t i2c;
    rcc_periph_clken rcc;
};

static constexpr i2c_def i2cs[] = {{I2C1, RCC_I2C1}, {I2C2, RCC_I2C2}};
} // namespace detail

class twim
    : public self_pointing<twim>
    , public tracked_driver<twim, 2>
    , non_copy_movable {
public:
    twim(gpio::pin_type clk, gpio::pin_type data)
        : tracked_driver(0) {
        m_def = detail::i2cs + 0;

        rcc_periph_clock_enable(m_def->rcc);

        i2c_reset(m_def->i2c);

        rcc_periph_clock_enable(data.port->rcc);
        rcc_periph_clock_enable(clk.port->rcc);

        gpio_set_mode(data.port->which,
                      GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
                      data.pin);

        gpio_set_mode(clk.port->which,
                      GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
                      clk.pin);

        i2c_peripheral_disable(m_def->i2c);

        i2c_set_clock_frequency(m_def->i2c, I2C_CR2_FREQ_36MHZ);
        i2c_set_ccr(m_def->i2c, 0x1e);
        i2c_set_trise(m_def->i2c, 0x0b);
        i2c_set_own_7bit_slave_address(m_def->i2c, 0xab);

        i2c_peripheral_enable(m_def->i2c);
    }

    twi_tx_res transmit(twi_addr_t to, span<const char> buf) noexcept;

    twi_rx_res receive(twi_addr_t from, span<char> buf) noexcept;

private:
    const detail::i2c_def* m_def;
};
} // namespace stm32
} // namespace tos

namespace tos {
namespace stm32 {
inline tos::twi_tx_res tos::stm32::twim::transmit(tos::twi_addr_t to,
                                                  tos::span<const char> buf) noexcept {
    int32_t wait = 0;
    while ((I2C_SR2(m_def->i2c) & I2C_SR2_BUSY)) {
        if (wait++ > 10'000) {
            return tos::twi_tx_res::other;
        }
    }

    i2c_send_start(m_def->i2c);

    wait = 0;
    while (!((I2C_SR1(m_def->i2c) & I2C_SR1_SB) &
             (I2C_SR2(m_def->i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY)))) {
        if (wait++ > 10'000) {
            i2c_send_stop(m_def->i2c);
            return tos::twi_tx_res::other;
        }
    }

    i2c_send_7bit_address(m_def->i2c, to.addr, I2C_WRITE);

    wait = 0;
    // wait for address ack
    while (!(I2C_SR1(m_def->i2c) & I2C_SR1_ADDR)) {
        if (wait++ > 10'000) {
            i2c_send_stop(m_def->i2c);
            return tos::twi_tx_res::addr_nack;
        }
    }

    (void)I2C_SR2(m_def->i2c);

    for (char c : buf) {
        i2c_send_data(m_def->i2c, c);

        wait = 0;
        while (!(I2C_SR1(m_def->i2c) & (I2C_SR1_BTF))) {
            if (wait++ > 10'000) {
                i2c_send_stop(m_def->i2c);
                return tos::twi_tx_res::data_nack;
            }
        }
    }

    i2c_send_stop(m_def->i2c);

    return tos::twi_tx_res::ok;
}

twi_rx_res twim::receive(twi_addr_t from, span<char> buf) noexcept {
    i2c_send_start(m_def->i2c);
    i2c_enable_ack(m_def->i2c);

    int wait = 0;
    /* Wait for master mode selected */
    while (!((I2C_SR1(m_def->i2c) & I2C_SR1_SB) &
             (I2C_SR2(m_def->i2c) & (I2C_SR2_MSL | I2C_SR2_BUSY)))) {
        if (wait++ > 10'000) {
            i2c_send_stop(m_def->i2c);
            return tos::twi_rx_res::other;
        }
    }

    i2c_send_7bit_address(m_def->i2c, from.addr, I2C_READ);
    wait = 0;
    /* Waiting for address is transferred. */
    // wait for address ack
    while (!(I2C_SR1(m_def->i2c) & I2C_SR1_ADDR)) {
        if (wait++ > 10'000) {
            i2c_send_stop(m_def->i2c);
            return tos::twi_rx_res::addr_nack;
        }
    }

    /* Clearing ADDR condition sequence. */
    (void)I2C_SR2(m_def->i2c);

    for (size_t i = 0; i < buf.size(); ++i) {
        if (i == buf.size() - 1) {
            i2c_disable_ack(m_def->i2c);
        }
        wait = 0;
        while (!(I2C_SR1(m_def->i2c) & I2C_SR1_RxNE)) {
            if (wait++ > 10'000) {
                i2c_send_stop(m_def->i2c);
                return tos::twi_rx_res::data_nack;
            }
        }
        buf[i] = i2c_get_data(m_def->i2c);
    }
    i2c_send_stop(m_def->i2c);

    return tos::twi_rx_res::ok;
}
} // namespace stm32
} // namespace tos
