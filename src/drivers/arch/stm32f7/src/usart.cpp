//
// Created by fatih on 12/5/18.
//

#include <arch/usart.hpp>

using tos::stm32::usart;

extern "C" void USART1_IRQHandler() {
    auto ins = usart::get(0);
    ins->isr();
}

extern "C" void USART2_IRQHandler() {
    auto ins = usart::get(1);
    ins->isr();
}

extern "C" void USART3_IRQHandler() {
    auto ins = usart::get(2);
    ins->isr();
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart) {
    usart* ins{};
    switch (intptr_t(huart->Instance)) {
    case USART1_BASE:
        ins = usart::get(0);
        break;
    case USART2_BASE:
        ins = usart::get(1);
        break;
    case USART3_BASE:
        ins = usart::get(2);
        break;
    }
    ins->tx_done_isr();
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    usart* ins{};
    switch (intptr_t(huart->Instance)) {
    case USART1_BASE:
        ins = usart::get(0);
        break;
    case USART2_BASE:
        ins = usart::get(1);
        break;
    case USART3_BASE:
        ins = usart::get(2);
        break;
    }
    ins->rx_done_isr();
}