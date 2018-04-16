//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/spi.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <avr/interrupt.h>

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

namespace tos
{
namespace sd
{
    void sd_delay()
    {
        //avr::spi_guard tr;

        for (int i = 0; i < 10; ++i)
        {
            avr::spi_put_byte(0xFF);
        }
    }

    void sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc, uint8_t read)
    {
        uint8_t buf[8];
        {
            avr::spi_guard tr;

            avr::spi_put_byte(cmd);
            avr::spi_put_byte(arg >> 24); // MSB first
            avr::spi_put_byte(arg >> 16);
            avr::spi_put_byte(arg >> 8);
            avr::spi_put_byte(arg);
            avr::spi_put_byte(crc);

            for (auto i = 0; i < read; ++i)
            {
                buf[i] = avr::spi_put_byte(0xFF);
            }
        }

        for (auto i = 0; i < read; ++i)
        {
            print_hex(buf[i]);
            print(comm, " ");
        }
        println(comm, "");
    }
}
}

void main_task()
{
    tos::usart0::set_baud_rate(9600);
    tos::usart0::set_2x_rate();
    tos::usart0::set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    tos::usart0::enable();

    println(comm, "Hi from master!");

    tos::avr::spi0::init_master();
    tos::avr::spi0::enable();
    tos::sd::sd_delay();
    while (true)
    {
        auto c = comm.getc();
        switch (c)
        {
        case '1':
            tos::sd::sd_cmd(0x40, 0x00000000, 0x95, 8);
            break;
        case '2':
            tos::sd::sd_cmd(0x41, 0x00000000, 0xFF, 8);
            break;
        case '3':
            tos::sd::sd_cmd(0x50, 0x00000200, 0xFF, 8);
            break;
        case '4':
            tos::sd::sd_cmd(0x48, 0x000001AA, 0x87, 8);
            break;
        case '5':
        tos::sd::sd_cmd(0x40 + 55, 0x0, 0xFF, 8);
        break;
        case '6':
        tos::sd::sd_cmd(0x41)
        }
    }
}

int main()
{
    ft::start(main_task);
    sei();

    while(true)
    {
        ft::schedule();
    }
}