//
// Created by fatih on 12/5/18.
//

#include <usart.hpp>

bool wait;
void usart2_isr()
{
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
