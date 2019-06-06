//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <arch/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <common/sd/spi_sd.hpp>
#include <common/gpio.hpp>
#include <common/spi.hpp>
#include <common/alarm.hpp>
#include <tos/devices.hpp>
#include <tos/waitable.hpp>
#include <stdlib.h>
#include <tos/semaphore.hpp>

template <class StreamT>
void print_hex(StreamT& c, unsigned char n) {
    if(((n>>4) & 15) < 10)
        tos::print(c, '0' + ((n>>4)&15));
    else
        tos::print(c, 'A' + ((n>>4)&15) - 10);
    n <<= 4;
    if(((n>>4) & 15) < 10)
        tos::print(c, '0' + ((n>>4)&15));
    else
        tos::print(c, 'A' + ((n>>4)&15) - 10);
}

void main_task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    auto g = open(tos::devs::gpio);

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi.enable();

    auto sd = open(tos::devs::sd, spi, g, 10_pin);
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
        char c[1];
        auto cmd = usart.read(c)[0];
        switch (cmd)
        {
        case '6':
        {
            auto blk = usart.read(c)[0] - '0';
            auto off = usart.read(c)[0] - '0';
            sd.read(buf, blk, 20, off);
            for (auto c : buf) {
                print_hex(usart, c);
                tos::println(*usart, " ");
            }
            tos::println(*usart, "");
            break;
        }
        }
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, main_task);
}