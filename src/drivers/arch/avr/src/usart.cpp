//
// Created by fatih on 4/11/18.
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <usart.hpp>
#include <ft/include/tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/mutex.hpp>

namespace tos {
    namespace avr {
        static void set_raw_baud_rate(uint16_t baud) {
            const uint16_t prescale = F_CPU / (baud * 16UL) - 1;
            UBRR0L = (uint8_t) (prescale & 0xff);
            UBRR0H = (uint8_t) (prescale >> 8);
        }

        static void set_2x_rate() {
            UCSR0A |= (1 << U2X0);
        }

        void usart0::set_baud_rate(usart_baud_rate baud) {
            set_2x_rate();
            set_raw_baud_rate(baud.rate / 2);
        }

        void usart0::enable() {
            UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0);
        }

        void usart0::disable() {
            UCSR0B &= ~((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0));
        }

        void usart0::options(usart_modes m, usart_parity p, usart_stop_bit s) {
            UCSR0C = usart_control(m, p, s);
        }
    }
}

template<class T, size_t len>
struct ringbuf {
public:
    ringbuf() : m_begin(0), m_end(0) {}

    size_t capacity() const { return len; }

    void push_back(const T &t) {
        m_store[m_end] = t;
        m_end = (m_end + 1) % len;
    }

    void pop_front() {
        m_begin = (m_begin + 1) % len;
    }

    T &front() {
        return m_store[m_begin];
    }

private:
    T m_store[len];
    size_t m_begin, m_end;
};

struct open_state {
    /**
     * Synchronizes the availabilty of the write buffer
     */
    tos::mutex write_avail;
    const char *volatile write = nullptr;
    uint16_t len = 0;

    tos::semaphore write_done{0};

    ringbuf<char, 32> read_buf;
    tos::semaphore have_data{0};
};

static open_state *create() {
    static open_state state;
    return &state;
}

open_state *state = create();

static void put_sync(const char data)
{
    while (!(UCSR0A & (1<<UDRE0)));
    UDR0 = data;
}

namespace tos
{
    namespace avr
    {
        void write_sync(const char* x, size_t len)
        {
            while (*x)
            {
                put_sync(*x++);
            }
        }
    }
}

static void write_usart(const char* x, size_t len)
{
    tos::lock_guard<tos::mutex> lg{state->write_avail};

    state->len = len;
    state->write = x;
    tos::busy(); // We'll block, keep mcu awake
    UCSR0B |= (1 << UDRIE0); // enable empty buffer interrupt, ISR will send the data
    state->write_done.down();
    tos::unbusy();
}

static size_t read_usart(char *buf, size_t len) {
    size_t total = 0;
    while (state && total < len) {
        state->have_data.down();
        *buf = state->read_buf.front();
        state->read_buf.pop_front();
        ++buf;
        ++total;
    }
    return total;
}

namespace tos
{
    int usart::read(char *buf, size_t sz) {
        return read_usart(buf, sz);
    }

    char usart::getc() {
        char x;
        read(&x, 1);
        return x;
    }

    int usart::write(const char *buf, size_t sz) {
        write_usart(buf, sz);
        return sz - state->len;
    }

    void usart::putc(char c) {
        write(&c, 1);
    }
}

ISR (USART_TX_vect)
{
    state->write_done.up_isr();
}

ISR (USART_UDRE_vect) {
    if (state->len == 0) {
        UCSR0B &= ~(1 << UDRIE0);
        return;
    }

    UDR0 = *state->write;
    ++state->write;

    state->len--;
}

ISR (USART_RX_vect) {
    auto byte = UDR0;
    state->read_buf.push_back(byte);
    state->have_data.up_isr();
}
