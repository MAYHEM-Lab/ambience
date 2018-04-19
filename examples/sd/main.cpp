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

template <class T>
class software_timer
{
public:
    void sleep(uint32_t microsecs)
    {

    }

private:
    T m_hw_timer;
    tos::intrusive_list<tos::thread_info> sleepers;
};

void tick_task()
{
    auto tmr = open(tos::devs::timer<1>);
    //tmr->disable();
    tmr->set_frequency(1000);
    tmr->enable();

    uint16_t ticks = 0;
    while (true)
    {
        tmr->block();
        ticks++;
        if (ticks == 1000)
        {
            println(comm, "Tick!", (int32_t)tmr->get_ticks());
            ticks = 0;
        }
    }
}

void main_task()
{
    auto usart = open(tos::devs::usart<0>);
    usart->set_baud_rate(19200);
    usart->set_control(tos::usart_modes::async, tos::usart_parity::disabled, tos::usart_stop_bit::one);
    usart->enable();

    println(comm, "Hi from master!");

    auto spi = open(tos::devs::spi<0>, tos::spi_mode::master);
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

    auto tmr = open(tos::devs::timer<1>);
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
            println(comm, tmr->get_ticks());
            break;
        }
        }
    }
}

int main()
{
    tos::launch(main_task);
    tos::launch(tick_task);
    tos::enable_interrupts();

    while(true)
    {
        tos::schedule();
    }
}