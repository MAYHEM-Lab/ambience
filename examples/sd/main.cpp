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
#include <tos/devices.hpp>

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

struct
{
    static constexpr const char* vendor()
    {
        return "atmel";
    }
    static constexpr const char* name()
    {
        return "atmega328";
    }
} device_descr;

void main_task()
{
    auto usart = open(tos::devs::usart<0>);
    usart->set_baud_rate(19200);
    usart->set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    usart->enable();

    println(comm, device_descr.vendor());
    println(comm, device_descr.name());
    println(comm, "Hi from master!");

    auto spi = open(tos::devs::spi<0>);
    spi->init_master();
    spi->enable();

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