//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/spi.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <avr/interrupt.h>
#include <avr/delay.h>

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
        //avr::spi_transaction tr;

        for (int i = 0; i < 10; ++i)
        {
            avr::spi_put_byte(0xFF);
        }
    }

    avr::spi_transaction exec_cmd(uint8_t cmd, uint32_t arg, uint8_t crc = 0xFF)
    {
        avr::spi_transaction tr;

        avr::spi_put_byte(cmd);
        avr::spi_put_byte(arg >> 24); // MSB first
        avr::spi_put_byte(arg >> 16);
        avr::spi_put_byte(arg >> 8);
        avr::spi_put_byte(arg);
        avr::spi_put_byte(crc);

        return tr;
    }

    uint8_t read_8(avr::spi_transaction&& tr)
    {
        uint8_t ret = 0xFF;
        uint8_t buf[8];
        for (int i = 0; i < 8; ++i)
        {
            buf[i] = avr::spi_put_byte(0xFF);
            if (buf[i] != 0xFF) ret = buf[i];
        }

        print_hex(ret);
        return ret;
    }

    void read_sector(avr::spi_transaction&& tr)
    {
        while (avr::spi_put_byte(0xFF) != 0x00);
        while (avr::spi_put_byte(0xFF) != 0xFE);
        for (int i = 0; i < 512; ++i)
        {
            print(comm, (char)avr::spi_put_byte(0xFF));
        }
        avr::spi_put_byte(0xFF);
        avr::spi_put_byte(0xFF);
        avr::spi_put_byte(0xFF);
    }

    bool init_sd()
    {
        int i;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x40, 0x00000000, 0x95)) == 0x01) break;
            _delay_ms(100);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x48, 0x000001AA, 0x87)) == 0xAA) break;
            _delay_ms(100);
        }
        if (i == 10) return false;
        for (i = 0; i < 10; ++i)
        {
            if (read_8(exec_cmd(0x40 + 55, 0x0)) != 0xFF && read_8(exec_cmd(0x69, 0x40000000)) == 0x00) break;
            _delay_ms(100);
        }
        return i != 10;
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

    if (!tos::sd::init_sd())
    {
        println(comm, "fuck");
    }
    else
    {
        println(comm, "ready");
    }

    using namespace tos::sd;
    read_8(tos::sd::exec_cmd(0x50, 0x00000200, 0xFF));
    while (true)
    {
        auto c = comm.getc();
        switch (c)
        {
        case '5':
            read_sector(exec_cmd(0x51, (comm.getc() - '0') * 512));
            break;
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