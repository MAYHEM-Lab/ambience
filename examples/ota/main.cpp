//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <tos/devices.hpp>
#include <util/delay.h>
#include <avr/boot.h>
#include <tos/interrupt.hpp>
#include <avr/io.h>

tos::usart comm;

void BOOTLOADER_SECTION foo()
{
    println(comm, "Yo from foo");
}

void tick_task()
{
    using namespace tos::tos_literals;
    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    println(comm, "Hello");
    println(comm, (void*)&tick_task);
    println(comm, (void*)&foo);
    while (true)
    {
        comm.getc();
        foo();
    }
}

int main()
{
    tos::enable_interrupts();

    tos::launch(tick_task);

    while(true)
    {
        tos::schedule();
    }
}