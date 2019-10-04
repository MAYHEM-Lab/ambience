//
// Created by fatih on 11/29/18.
//

#pragma once

#include "gpio.hpp"

#include <common/driver_base.hpp>
#include <common/usart.hpp>
#include <stm32_hal/usart.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ring_buf.hpp>

namespace tos {
namespace stm32 {
namespace detail {
struct usart_def {
    USART_TypeDef* usart;
    IRQn_Type irq;
    void (*rcc_en)();
    void (*rcc_dis)();
};

inline const usart_def usarts[] = {
    {USART1, USART1_IRQn, [] { __HAL_RCC_USART1_CLK_ENABLE(); }, [] { __HAL_RCC_USART1_CLK_DISABLE(); }},
    {USART2, USART2_IRQn, [] { __HAL_RCC_USART2_CLK_ENABLE(); }, [] { __HAL_RCC_USART2_CLK_DISABLE(); }},
#if defined(USART3)
    {USART3, USART3_IRQn, [] { __HAL_RCC_USART3_CLK_ENABLE(); }, [] { __HAL_RCC_USART3_CLK_DISABLE(); }},
#endif
};
} // namespace detail

using usart_constraint = ct_map<usart_key_policy,
                                el_t<usart_baud_rate, const usart_baud_rate&>,
                                el_t<usart_parity, const usart_parity&>,
                                el_t<usart_stop_bit, const usart_stop_bit&>>;

class usart
    : public self_pointing<usart>
    , public tracked_driver<usart, std::size(detail::usarts)>
    , public non_copyable {
public:
    explicit usart(const detail::usart_def&,
                   usart_constraint&&,
                   gpio::pin_type rx,
                   gpio::pin_type tx);

    int write(tos::span<const uint8_t> buf);

    int write(tos::span<const char> buf) {
        return write(tos::raw_cast<const uint8_t>(buf));
    }

    tos::span<char> read(tos::span<char> b);

    template<class AlarmT>
    tos::span<char> read(tos::span<char> b, AlarmT& alarm, std::chrono::milliseconds to);

    ~usart() {
        NVIC_DisableIRQ(m_def->irq);
        HAL_UART_DeInit(&m_handle);
        m_def->rcc_dis();
    }

    void isr() {
        HAL_UART_IRQHandler(&m_handle);
    }

    void tx_done_isr() {
        tx_s.up_isr();
    }

    void rx_done_isr() {
        rx_buf.push(m_recv_byte);
        rx_s.up_isr();

        HAL_UART_Receive_IT(&m_handle, &m_recv_byte, 1);
    }

    UART_HandleTypeDef* native_handle() {
        return &m_handle;
    }

private:
    tos::fixed_fifo<uint8_t, 32, tos::ring_buf> rx_buf;
    tos::semaphore rx_s{0};
    tos::semaphore tx_s{0};

    uint8_t m_recv_byte;
    UART_HandleTypeDef m_handle;
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
                    gpio::pin_type rx_pin,
                    gpio::pin_type tx_pin)
    : tracked_driver(std::distance(detail::usarts, &x))
    , m_def{&x} {
    m_def->rcc_en();
    {
        enable_rcc(rx_pin.port);
        GPIO_InitTypeDef init{};
        init.Pin = rx_pin.pin;
        init.Mode = GPIO_MODE_AF_OD;
        init.Pull = GPIO_NOPULL;
        init.Speed = detail::gpio_speed::highest();
#if !defined(STM32F1)
        init.Alternate = GPIO_AF4_USART1;
#endif
        HAL_GPIO_Init(rx_pin.port, &init);
    }

    {
        enable_rcc(tx_pin.port);
        GPIO_InitTypeDef init{};
        init.Pin = tx_pin.pin;
        init.Mode = GPIO_MODE_AF_PP;
        init.Pull = GPIO_NOPULL;
        init.Speed = detail::gpio_speed::highest();
#if !defined(STM32F1)
        init.Alternate = GPIO_AF4_USART1;
#endif
        HAL_GPIO_Init(tx_pin.port, &init);
    }

    m_handle = {};
    m_handle.Instance = m_def->usart;
    m_handle.Init.BaudRate = tos::get<usart_baud_rate>(params).rate;
    m_handle.Init.WordLength = UART_WORDLENGTH_8B;
    m_handle.Init.StopBits = UART_STOPBITS_1;
    m_handle.Init.Parity = UART_PARITY_NONE;
    m_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    m_handle.Init.Mode = UART_MODE_TX_RX;

    HAL_UART_Init(&m_handle);

    HAL_NVIC_SetPriority(m_def->irq, 0, 0);
    HAL_NVIC_EnableIRQ(m_def->irq);

    HAL_UART_Receive_IT(&m_handle, &m_recv_byte, 1);
}

template<class AlarmT>
tos::span<char>
usart::read(tos::span<char> b, AlarmT& alarm, std::chrono::milliseconds to) {
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

inline tos::span<char> usart::read(tos::span<char> b) {
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

inline int usart::write(tos::span<const uint8_t> buf) {
    if (buf.empty())
        return 0;
    auto res = HAL_UART_Transmit_IT(&m_handle, const_cast<uint8_t*>(buf.data()), buf.size());
    if (res != HAL_OK)
    {
        return -1;
    }
    tx_s.down();
    return buf.size();
}
} // namespace tos::stm32