//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos_arch.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

#include <usart.hpp>

#include <avr/interrupt.h>
#include <tos/intrusive_ptr.hpp>

ft::semaphore timer_sem(0);
void send_usart(const char *x);

void hello_task()
{
    tos::gpio gp;
    gp.set_pin_mode(13, tos::gpio::pin_mode_t::out);
    char buf[3];
    buf[1] = 0;
    while (true) {
        //timer_sem.down();
        read_usart(buf, 1);
        gp.write(13, true);
        send_usart(buf);
        //send_usart(" On\n");

        //timer_sem.down();
        read_usart(buf, 1);
        gp.write(13, false);
        send_usart(buf);
        //send_usart(" Off\n");
    }
}

void write_task();

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
    using namespace tos;

    tos::avr_usart0::set_baud_rate(9600);
    //tos::avr_usart0::set_2x_rate();
    tos::avr_usart0::set_control(usart_modes::async, usart_parity::disabled, usart_stop_bit::one);

    tos::avr_usart0::enable();
}

int main()
{
    ft::start(hello_task);
    ft::start(write_task);
    timer1_init();
    initUSART();
    sei();

    while (true)
    {
        ft::schedule();
    }
}