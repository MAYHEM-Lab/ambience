//
// Created by fatih on 4/11/18.
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <usart.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/mutex.hpp>

namespace tos {
    void avr_usart0::set_baud_rate(uint16_t baud) {
        const uint16_t prescale = F_CPU / (baud * 16UL) - 1;
        UBRR0L = (uint8_t) (prescale & 0xff);
        UBRR0H = (uint8_t) (prescale >> 8);
    }

    void avr_usart0::set_2x_rate() {
        UCSR0A |= (1 << U2X0);
    }

    void avr_usart0::enable() {
        UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);// | (1 << TXCIE0);
    }

    void avr_usart0::disable() {
        UCSR0B &= ~((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0));
    }

    void avr_usart0::set_control(usart_modes m, usart_parity p, usart_stop_bit s) {
        UCSR0C = usart_control(m, p, s);
    }
}

template <class T, size_t len>
struct ringbuf
{
public:
    ringbuf() : m_begin(0), m_end(0) {}
    size_t capacity() const { return len; }

    void push_back(const T& t){
        m_store[m_end] = t;
        m_end = (m_end + 1) % len;
    }

    void pop_front()
    {
        m_begin = (m_begin + 1) % len;
    }

    T& front()
    {
        return m_store[m_begin];
    }

private:
    T m_store[len];
    size_t m_begin, m_end;
};

struct open_state
{
    /**
     * Synchronizes the availabilty of the write buffer
     */
    ft::semaphore have_write{1};
    const char* volatile write = nullptr;

    ft::semaphore write_done{0};

    ringbuf<char, 32> read_buf;
    ft::semaphore have_data{0};
};

open_state* create()
{
    static open_state state;
    return &state;
}

open_state* state = create();

void write_usart(const char *x)
{
    state->have_write.down();

    state->write = x;
    UCSR0B |= (1<<UDRIE0); // enable empty buffer interrupt, ISR will send the data
    state->write_done.down();

    state->have_write.up();
}

size_t read_usart(char* buf, size_t len)
{
    size_t total = 0;
    while (state && total < len)
    {
        state->have_data.down();
        *buf = state->read_buf.front();
        state->read_buf.pop_front();
        ++buf;
        ++total;
    }
    return total;
}

ISR (USART_UDRE_vect) {
    UDR0 = *state->write;
    ++state->write;

    if (!*state->write) {
        UCSR0B &= ~(1 << UDRIE0);
        state->write_done.up();
    }
}

ISR (USART_RX_vect) {
    auto byte = UDR0;
    state->read_buf.push_back(byte);
    state->have_data.up();
}
