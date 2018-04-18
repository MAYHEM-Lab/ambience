//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/spi.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/spi_sd.hpp>
#include <drivers/gpio.hpp>

tos::usart comm;
void print_hex(unsigned char n) {
    if(((n>>4) & 15) < 10)
        comm.putc('0' + ((n>>4)&15));
    else
        comm.putc('A' + ((n>>4)&15) - 10);
    n <<= 4;
    if(((n>>4) & 15) < 10)
        comm.putc('0' + ((n>>4)&15));
    else
        comm.putc('A' + ((n>>4)&15) - 10);
}

void main_task()
{
    tos::avr::usart0::set_baud_rate(9600);
    tos::avr::usart0::set_2x_rate();
    tos::avr::usart0::set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    tos::avr::usart0::enable();

    println(comm, "Hi from master!");

    tos::avr::spi0::init_master();
    tos::avr::spi0::enable();

    tos::spi_sd_card sd{2};
    if (!sd.init())
    {
        println(comm, "that didn't work");
    }
    else
    {
        println(comm, "ready");
    }

    uint8_t buf[20];
    while (true)
    {
        auto cmd = comm.getc();
        switch (cmd)
        {
        case '6':
        {
            auto blk = comm.getc() - '0';
            auto off = comm.getc() - '0';
            sd.read(buf, blk, 20, off);
            for (auto c : buf) {
                print_hex(c);
                print(comm, " ");
            }
            println(comm, "");
            break;
        }
        }
    }
}

int main()
{
    tos::launch(main_task);
    tos::enable_interrupts();

    while(true)
    {
        tos::schedule();
    }
}