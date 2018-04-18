//
// Created by fatih on 4/15/18.
//

#include <spi.hpp>
#include <tos/ft.hpp>
#include <usart.hpp>
#include <tos/print.hpp>
#include <avr/interrupt.h>

void master_task()
{
    tos::avr::spi0::init_master();
    tos::avr::spi0::enable();

    tos::usart0::set_baud_rate(9600);
    tos::usart0::set_2x_rate();
    tos::usart0::set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    tos::usart0::enable();
    tos::usart comm;

    println(comm, "Hi from master!");

    while (true)
    {
        char c = comm.getc();
        tos::avr::spi_put_byte(c);
    }
}

int main()
{
    tos::launch(master_task);
    sei();

    while(true)
    {
        tos::schedule();
    }
}