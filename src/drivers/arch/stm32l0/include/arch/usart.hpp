//
// Created by fatih on 11/29/18.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <tos/fixed_fifo.hpp>
#include <tos/ring_buf.hpp>

template<int num, uint32_t BASE>
void usart_isr();

namespace tos {
namespace stm32 {
namespace detail {
struct usart_def {
    uint32_t usart;
    rcc_periph_clken clk;
    rcc_periph_rst rst;
    uint8_t irq;
};

static constexpr usart_def usarts[] = {
    {USART1, RCC_USART1, RST_USART1, NVIC_USART1_IRQ},
    {USART2, RCC_USART2, RST_USART2, NVIC_USART2_IRQ},
};
} // namespace detail

using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class usart
    : public self_pointing<usart>
    , public tracked_driver<usart, 3>
    , public non_copyable {
public:
    explicit usart(const detail::usart_def&,
                   usart_constraint&&,
                   gpio::pin_type rx,
                   gpio::pin_type tx);

    int write(tos::span<const uint8_t> buf) {
        if (buf.size() == 0)
            return 0;
        tx_it = buf.begin();
        tx_end = buf.end();
        USART_CR1(m_def->usart) |= USART_CR1_TXEIE;
        tx_s.down();
        return buf.size();
    }

    int write(tos::span<const char> buf) {
        return write(tos::span<const uint8_t>{(const uint8_t*)buf.data(), buf.size()});
    }

    tos::span<char> read(tos::span<char> b) {
        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        tos::kern::busy();
        while (total < len) {
            rx_s.down();
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        tos::kern::unbusy();
        return b.slice(0, total);
    }

    template<class AlarmT>
    tos::span<char> read(tos::span<char> b, AlarmT& alarm, std::chrono::milliseconds to) {
        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        tos::kern::busy();
        while (total < len) {
            auto res = rx_s.down(alarm, to);
            if (res == sem_ret::timeout) {
                break;
            }
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        tos::kern::unbusy();
        return b.slice(0, total);
    }

    ~usart() {
        usart_disable(m_def->usart);
        usart_disable_rx_interrupt(m_def->usart);
        nvic_disable_irq(m_def->irq);
        rcc_periph_clock_disable(m_def->clk);
    }

private:
    tos::fixed_fifo<uint8_t, 32, tos::ring_buf> rx_buf;
    const uint8_t* tx_it;
    const uint8_t* tx_end;
    tos::semaphore rx_s{0};
    tos::semaphore tx_s{0};
    template<int num, uint32_t BASE>
    friend void ::usart_isr();

    const detail::usart_def* m_def;
};
} // namespace stm32

inline stm32::usart open_impl(tos::devs::usart_t<1>,
                              stm32::usart_constraint&& constraints,
                              stm32::gpio::pin_type rx,
                              stm32::gpio::pin_type tx) {
    return stm32::usart{stm32::detail::usarts[0], std::move(constraints), rx, tx};
}

inline stm32::usart open_impl(tos::devs::usart_t<2>,
                              stm32::usart_constraint&& constraints,
                              stm32::gpio::pin_type rx,
                              stm32::gpio::pin_type tx) {
    return stm32::usart{stm32::detail::usarts[1], std::move(constraints), rx, tx};
}

inline stm32::usart open_impl(tos::devs::usart_t<3>,
                              stm32::usart_constraint&& constraints,
                              stm32::gpio::pin_type rx,
                              stm32::gpio::pin_type tx) {
    return stm32::usart{stm32::detail::usarts[2], std::move(constraints), rx, tx};
}
} // namespace tos

// impl

namespace tos::stm32 {
inline usart::usart(const detail::usart_def& x,
                    usart_constraint&& params,
                    gpio::pin_type rx,
                    gpio::pin_type tx)
    : tracked_driver(std::distance(detail::usarts, &x))
    , m_def{&x} {
    rcc_periph_reset_pulse(x.rst);
    rcc_periph_clock_enable(x.clk);

    usart_set_baudrate(x.usart, tos::get<usart_baud_rate>(params).rate);
    usart_set_databits(x.usart, 8);
    usart_set_stopbits(x.usart, USART_STOPBITS_1);
    usart_set_mode(x.usart, USART_MODE_TX_RX);
    usart_set_parity(x.usart, USART_PARITY_NONE);
    usart_set_flow_control(x.usart, USART_FLOWCONTROL_NONE);

    gpio g;
    rcc_periph_clock_enable(tx.port->rcc);
    gpio_mode_setup(tx.port->which, GPIO_MODE_AF, GPIO_PUPD_NONE, tx.pin);
    gpio_set_af(tx.port->which, GPIO_AF7, tx.pin);

    rcc_periph_clock_enable(rx.port->rcc);
    g.set_pin_mode(rx, tos::pin_mode::in);
    gpio_set_af(rx.port->which, GPIO_AF7, rx.pin);

    nvic_enable_irq(x.irq);
    usart_enable_rx_interrupt(x.usart);
    usart_enable(x.usart);
}
} // namespace tos::stm32