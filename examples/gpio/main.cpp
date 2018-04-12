//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos_arch.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

#include <avr/interrupt.h>

ft::semaphore timer_sem(0);

ft::semaphore have_write{0};
const char* volatile data = nullptr;

void sent_usart(const char* x)
{
    data = x;
    have_write.up();
}

void hello_task()
{
    tos::gpio gp;
    gp.set_pin_mode(13, tos::gpio::pin_mode_t::out);
    while (true) {
        timer_sem.down();
        gp.write(13, true);
        sent_usart("On\n");

        timer_sem.down();
        gp.write(13, false);
        sent_usart("Off\n");
    }
}

ft::semaphore tx_comp{1};

void write_task()
{
    while (true)
    {
        have_write.down();

        UCSR0B |= (1<<UDRIE0);
        /*while (*data)
        {
            tx_comp.down();
            UDR0 = *data;
            ++data;
        }*/
    }
}

ISR (USART_UDRE_vect)
{
    UDR0 = *data;
    ++data;

    if (!*data)
    {
        UCSR0B &= ~(1<<UDRIE0);
    }
}

ISR (USART_TX_vect)
{
    tx_comp.up();
}

ISR (USART_RX_vect)
{
    if (UCSR0A);
    auto byte = UDR0;
    byte = byte;
    timer_sem.up();
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
    sei();
}

void initUSART()
{
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
    /* 9600 baud */
    UBRR0L = (uint8_t)(BAUD_PRESCALE & 0xff);
    UBRR0H = (uint8_t)(BAUD_PRESCALE >> 8);
    UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);
    UCSR0C =
            /* no parity bit */
            (0 << UPM01) |
                    (0 << UPM00) |
                    /* asyncrounous USART */
                    (0 << UMSEL01) |
                    (0 << UMSEL00) |
                    /* one stop bit */
                    (0 << USBS0) |
                    /* 8-bits of data */
                    (1 << UCSZ01) |
                    (1 << UCSZ00);
    UCSR0A = (1 << U2X0);
}

int main()
{
    ft::start(hello_task);
    ft::start(write_task);
    //timer1_init();
    initUSART();
    sei();

    while (true)
    {
        ft::schedule();
    }
}