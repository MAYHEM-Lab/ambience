//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/spi.hpp>
#include <drivers/arch/avr/usart.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/spi_sd.hpp>
#include <drivers/common/gpio.hpp>
#include <drivers/common/spi.hpp>
#include <tos/devices.hpp>
#include <drivers/arch/avr/timer.hpp>
#include <tos/waitable.hpp>

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

int32_t x_ticks = 0;
uint16_t ticks = 0;

void tick_task()
{
    auto tmr = open(tos::devs::timer<1>);
    //tmr->disable();
    tmr->set_frequency(1000);
    tmr->enable();
    tmr->set_callback([](void* d)
    {
        x_ticks++;
    }, nullptr);

    while (true)
    {
        //println(comm, "Blocking...");
        tmr->block();
        //println(comm, "Hi!");
        ticks++;
        if (ticks == 1000)
        {
            println(comm, "Tick!");
            //println(comm, "Tick!", (int32_t)tmr->get_ticks());
            ticks = 0;
        }
    }
}

void main_task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    println(comm, "Hi from master!");

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
    spi->enable();

    auto sd = open(tos::devs::sd, 2_pin);
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
        case '1':
        {
            println(comm, "state:", (int32_t)ticks, (int32_t)x_ticks, (int32_t)tos::runnable_count());
            break;
        }
        }
    }
}

int main()
{
    tos::enable_interrupts();

    tos::launch(main_task);
    tos::launch(tick_task);

    while(true)
    {
        tos::schedule();
    }
}