//
// Created by Mehmet Fatih BAKIR on 04/05/2018.
//

#include <tvm/exec/executor.hpp>
#include <drivers/arch/avr/usart.hpp>

#include <tos/print.hpp>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>

#include <tos/semaphore.hpp>
#include <eeprom.hpp>
#include <drivers/common/eeprom.hpp>

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

static char tvm_stack[128];

static ptr_fetcher fetch;
static tos::semaphore wait{0};

static svm::vm_state state;
static tos::semaphore s_wait{0};

void tvm_task()
{
    while (true)
    {
        wait.down();

        tvm::vm_executor<ptr_fetcher, svm::vm_state, svm::ISA> exec(fetch);
        exec.m_state.stack_begin = (uint16_t*)tvm_stack;
        exec.m_state.stack_cur = (uint16_t*)tvm_stack;
        exec.m_state.stack_end = (uint16_t*)(tvm_stack + 128);
        exec.exec();

        state = exec.m_state;
        s_wait.up();
    }
}

static uint8_t prog[128] =
        {0x04, 0x00, 0x00, 0xA0, 0x04,
         0x20, 0x01, 0x40, 0x02, 0x02,
         0x04, 0x20, 0x01, 0x00, 0x02,
         0x02, 0x0A, 0x00, 0x00, 0x00,
         0x00};

void main_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    auto eeprom = open(tos::devs::eeprom<0>);
    eeprom->read(0, prog, 128);

    while (true)
    {
        char buffer[1];
        usart->read(buffer);

        if (buffer[0] == 'x')
        {
            fetch = ptr_fetcher{prog};
            wait.up();

            s_wait.down();
            for (auto& reg : state.registers)
            {
                tos::print(*usart, reg, " ");
            }
            tos::println(*usart, "");
        }
        else if (buffer[0] == 'p')
        {
            tos::println(*usart, "send");
            usart->read(buffer);
            if (buffer[0] > 128)
            {
                tos::println(*usart, "too large");
                continue;
            }
            tos::println(*usart, "ok");
            usart->read({ (char*)prog, buffer[0] });
            eeprom->write(0, prog, 128);
            tos::println(*usart, "okay");
        }
    }
}

int main() {
    tos::enable_interrupts();

    tos::launch(tvm_task);
    tos::launch(main_task);

    while(true)
    {
        tos::schedule();
    }
}