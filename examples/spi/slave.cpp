//
// Created by fatih on 4/15/18.
//

#include <spi.hpp>
#include <usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <avr/interrupt.h>

void slave_task()
{
    tos::avr::spi0::init_slave();
    tos::avr::spi0::enable();

    tos::usart0::set_baud_rate(9600);
    tos::usart0::set_2x_rate();
    tos::usart0::set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    tos::usart0::enable();
    tos::usart comm;

    println(comm, "Hi from slave!");

    while (true)
    {
        char c = tos::avr::spi_read_byte();
        comm.putc(c);
    }
}

int main()
{
    ft::start(slave_task);
    sei();

    while(true)
    {
        ft::schedule();
    }
}