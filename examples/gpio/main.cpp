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

        //UCSR0B |= (1<<UDRIE0);
        while (*data)
        {
            tx_comp.down();
            UDR0 = *data;
            ++data;
        }
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


namespace tos
{
    enum class usart_modes : uint8_t
    {
        async = 0,
        sync = 0b01,
        reserved = 0b10,
        spi_master = 0b11
    };

    enum class usart_parity : uint8_t
    {
        disabled = 0,
        reserved = 0b01,
        even = 0b10,
        odd = 0b11
    };

    enum class usart_stop_bit : uint8_t
    {
        one = 0b0,
        two = 0b1
    };

    constexpr uint8_t usart_control(usart_modes mode, usart_parity parity, usart_stop_bit stop)
    {
        return ((uint8_t)mode << 6) | ((uint8_t)parity << 4) | ((uint8_t)stop << 3) | (0b11 << 1);
    }

/**
 * This class manages the AVR USART0 device
 */
class avr_usart0
{
public:
    void set_baud_rate(uint16_t);
    void set_2x_rate(bool);
    void enable();
    void disable();

    void set_control(usart_modes, usart_parity, usart_stop_bit);
};
}

namespace tos
{
    void avr_usart0::set_baud_rate(uint16_t baud)
    {
        const auto prescale = F_CPU / (baud * 16UL) - 1;
        UBRR0L = (uint8_t)(prescale & 0xff);
        UBRR0H = (uint8_t)(prescale >> 8);
    }

    void avr_usart0::set_2x_rate(bool)
    {
        UCSR0A |= (1 << U2X0);
    }

    void avr_usart0::enable()
    {
        UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0);
    }

    void avr_usart0::disable()
    {
        UCSR0B &= ~((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0));
    }

    void avr_usart0::set_control(usart_modes m, usart_parity p, usart_stop_bit s)
    {
        UCSR0B = usart_control(m, p, s);
    }
}

tos::avr_usart0 usart;

void initUSART()
{
    using namespace tos;

    usart.set_baud_rate(9600);
    usart.set_2x_rate(true);
    usart.set_control(usart_modes::async, usart_parity::disabled, usart_stop_bit::one);

    usart.enable();
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