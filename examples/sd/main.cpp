//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/spi.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/sd/spi_sd.hpp>
#include <drivers/common/gpio.hpp>
#include <drivers/common/spi.hpp>
#include <tos/devices.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <tos/waitable.hpp>
#include <stdlib.h>
#include <drivers/common/alarm.hpp>

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

uint16_t ticks = 0;
int32_t x_ticks = 0;

void tick_task()
{
    auto tmr = open(tos::devs::timer<1>);
    tos::alarm<tos::remove_reference_t<decltype(*tmr)>> alarm{*tmr};

    while (true)
    {
        alarm.sleep_for({ 1000 });
        println(comm, "Tick!");
    }
}

void main_task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::avr::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    tos::println(*usart, "Hi from master!");

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi->enable();

    auto sd = open(tos::devs::sd, 10_pin);
    if (!sd.init())
    {
        tos::println(*usart, "that didn't work");
    }
    else
    {
        tos::println(*usart, "ready");
    }

    auto csd = sd.read_csd();
    char sbuf[32];
    tos::println(*usart, "we have ", ultoa(get_blk_count(csd), sbuf, 10), " blocks");

    uint8_t buf[20];
    while (true)
    {
        auto cmd = comm.getc();
        switch (cmd)
        {
        case '6':
        {
            auto blk = usart->getc() - '0';
            auto off = comm.getc() - '0';
            sd.read(buf, blk, 20, off);
            for (auto c : buf) {
                print_hex(c);
                tos::println(*usart, " ");
            }
            tos::println(*usart, "");
            break;
        }
        case '1':
        {
            tos::println(*usart, "state:", (int32_t)ticks, (int32_t)x_ticks);
            break;
        }
        }
    }
}

void tos_main()
{
    tos::launch(main_task);
    tos::launch(tick_task);
}