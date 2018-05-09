//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <ft/include/tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos_arch.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

#include <drivers/arch/avr/usart.hpp>

#include <avr/interrupt.h>
#include <tos/intrusive_ptr.hpp>
#include <drivers/arch/avr/gpio.hpp>

tos::semaphore timer_sem(0);
tos::usart usart;

void hello_task()
{
    using namespace tos::tos_literals;
    tos::avr::gpio gp;
    gp.set_pin_mode(13_pin, tos::pin_mode_t::out);
    auto p = 13_pin;
    println(usart, "Pin: ", (int)p.pin);
    gp.write(13_pin, false);
    char buf;
    while (true) {

        usart.read(&buf, 1);
        if (buf == '1')
        {
            gp.write(13_pin, true);
            println(usart, "On");
        }
        else
        {
            gp.write(13_pin, false);
            println(usart, "Off");
        }
    }
}

void tick_task()
{
    while (true)
    {
        timer_sem.down();
        println(usart, "Tick");
    }
}

int total = 0;
ISR(TIMER1_OVF_vect)
{
    total ++;
    if (total % 1000 == 0)
    {
        timer_sem.up();
    }
    TCNT1 = 49536;
}

void timer1_init()
{
    TCCR1A = 0;
    TCCR1B = (1 << CS10); // no pre scaler
    TIMSK1 |= (1 << TOIE1);
    // initialize counter
    TCNT1 = 0;
}

void initUSART()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();
}

int main()
{
    tos::enable_interrupts();

    tos::launch(hello_task);
    tos::launch(tick_task);

    timer1_init();
    initUSART();

    while (true)
    {
        tos::schedule();
    }
}