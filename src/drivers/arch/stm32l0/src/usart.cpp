//
// Created by fatih on 12/5/18.
//

#include <arch/usart.hpp>

template <int num, uint32_t BASE>
void usart_isr()
{
    static bool wait{};

    auto& ins = *tos::stm32::usart::get(num);

    if ((USART_CR1(BASE) & USART_CR1_RXNEIE) &&
        (USART_ISR(BASE) & USART_ISR_TXE)) {
        ins.rx_buf.push(usart_recv(BASE));
        ins.rx_s.up_isr();
    }

    if ((USART_CR1(BASE) & USART_CR1_TXEIE) &&
        (USART_ISR(BASE) & USART_ISR_TXE))
    {
        if (wait)
        {
            usart_disable_tx_interrupt(BASE);
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

void usart1_isr() { usart_isr<0, USART1>(); }
void usart2_isr() { usart_isr<0, USART2>(); }
