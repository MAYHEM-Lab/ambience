//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#include <tvm/exec/executor.hpp>
#include <drivers/arch/avr/usart.hpp>

#include <tos/print.hpp>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>

#include "vm_def.hpp"
#include "vm_state.hpp"

struct ptr_fetcher
{
    constexpr uint32_t fetch(uint16_t pc);
    uint8_t* m_base;
};

constexpr uint32_t ptr_fetcher::fetch(uint16_t pc)
{
    auto p = m_base + pc;
    uint32_t res = 0;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++; res <<= 8;
    res |= *p++;
    return res;
}

tos::usart comm;
void tvm_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();
    uint8_t data[] = {
            0x04, 0x00, 0x00, 0xA0, 0x04, 0x20, 0x01, 0x40, 0x02, 0x02, 0x04, 0x20, 0x01, 0x00, 0x02, 0x02,
            0x0A, 0x00, 0x00, 0x00, 0x00
    };
    ptr_fetcher fetch{data};

    tvm::vm_executor<ptr_fetcher, svm::vm_state, svm::ISA> exec(fetch);
    exec.exec();

    println(comm, "Result:", (int)exec.m_state.registers[0]);
}

int main() {
    tos::enable_interrupts();

    tos::launch(tvm_task);

    while(true)
    {
        tos::schedule();
    }
}