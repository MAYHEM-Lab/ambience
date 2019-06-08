//
// Created by fatih on 12/5/18.
//

#include <arch/usart.hpp>

void usart1_isr()
{
    static bool wait{};

    auto& ins = *tos::stm32::usart::get(0);

    if ((USART_CR1(USART1) & USART_CR1_RXNEIE) &&
        (USART_SR(USART1) & USART_SR_RXNE)) {
        ins.rx_buf.push(usart_recv(USART1));
        ins.rx_s.up_isr();
    }

    if ((USART_CR1(USART1) & USART_CR1_TXEIE) &&
        (USART_SR(USART1) & USART_SR_TXE))
    {
        if (wait)
        {
            usart_disable_tx_interrupt(USART1);
            ins.tx_s.up_isr();
            wait = false;
            return;
        }
        usart_send(USART1, *ins.tx_it++);
        if (ins.tx_it == ins.tx_end)
        {
            wait = true;
            return;
        }
    }
}

void usart2_isr()
{
    static bool wait{};

    auto& ins = *tos::stm32::usart::get(1);

    if ((USART_CR1(USART2) & USART_CR1_RXNEIE) &&
        (USART_SR(USART2) & USART_SR_RXNE)) {
        ins.rx_buf.push(usart_recv(USART2));
        ins.rx_s.up_isr();
    }

    if ((USART_CR1(USART2) & USART_CR1_TXEIE) &&
        (USART_SR(USART2) & USART_SR_TXE))
    {
        if (wait)
        {
            usart_disable_tx_interrupt(USART2);
            ins.tx_s.up_isr();
            wait = false;
            return;
        }
        usart_send(USART2, *ins.tx_it++);
        if (ins.tx_it == ins.tx_end)
        {
            wait = true;
            return;
        }
    }
}

void usart3_isr()
{
    static bool wait{};

    auto& ins = *tos::stm32::usart::get(2);

    if ((USART_CR1(USART3) & USART_CR1_RXNEIE) &&
        (USART_SR(USART3) & USART_SR_RXNE)) {
        ins.rx_buf.push(usart_recv(USART3));
        ins.rx_s.up_isr();
    }

    if ((USART_CR1(USART3) & USART_CR1_TXEIE) &&
        (USART_SR(USART3) & USART_SR_TXE))
    {
        if (wait)
        {
            usart_disable_tx_interrupt(USART3);
            ins.tx_s.up_isr();
            wait = false;
            return;
        }
        usart_send(USART3, *ins.tx_it++);
        if (ins.tx_it == ins.tx_end)
        {
            wait = true;
            return;
        }
    }
}

