//
// Created by fatih on 4/11/18.
//

#include <arch/usart.hpp>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <optional>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>
#include <tos/semaphore.hpp>

namespace tos {
namespace avr {

namespace {
template<class T, size_t len>
struct ringbuf {
public:
    ringbuf()
        : m_begin(0)
        , m_end(0) {
    }

    size_t capacity() const {
        return len;
    }

    void push_back(const T& t) {
        m_store[m_end] = t;
        m_end = (m_end + 1) % len;
    }

    void pop_front() {
        m_begin = (m_begin + 1) % len;
    }

    T& front() {
        return m_store[m_begin];
    }

    void clear() {
        m_begin = 0;
        m_end = 0;
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
    const uint8_t* volatile write = nullptr;
    uint16_t len = 0;

    tos::semaphore write_done{0};

    ringbuf<uint8_t, 32> read_buf;
    tos::semaphore have_data{0};
};

std::optional<open_state> state;

void write_usart(tos::span<const uint8_t> buf) {
    tos::lock_guard<tos::mutex> lg{state->write_avail};

    state->len = buf.size();
    state->write = buf.data();
    tos::kern::busy();       // We'll block, keep mcu awake
    UCSR0B |= (1 << UDRIE0); // enable empty buffer interrupt, ISR will send the data
    state->write_done.down();
    tos::kern::unbusy();
}

constexpr uint8_t
usart_control(usart_modes mode, usart_parity parity, usart_stop_bit stop) {
    // only support 8 bit chars atm
    return ((uint8_t)mode << 6) | ((uint8_t)parity << 4) | ((uint8_t)stop << 3) |
           (0b11 << 1);
}
} // namespace

static void set_raw_baud_rate(uint16_t baud) {
    const uint16_t prescale = F_CPU / (baud * 16UL) - 1;
    UBRR0L = (uint8_t)(prescale & 0xff);
    UBRR0H = (uint8_t)(prescale >> 8);
}

static void set_2x_rate() {
    UCSR0A |= (1 << U2X0);
}

void usart0::set_baud_rate(usart_baud_rate baud) {
    set_2x_rate();
    set_raw_baud_rate(baud.rate / 2);
}

void usart0::enable() {
    state.emplace();
    UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0);
}

void usart0::disable() {
    UCSR0B &= ~((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0) | (1 << TXCIE0));
    state.reset();
}

void usart0::options(usart_modes m, usart_parity p, usart_stop_bit s) {
    UCSR0C = usart_control(m, p, s);
}

span<uint8_t> usart0::read(span<uint8_t> b) {
    size_t total = 0;
    auto len = b.size();
    auto buf = b.data();
    tos::kern::busy();
    while (state && total < len) {
        state->have_data.down();
        *buf = state->read_buf.front();
        state->read_buf.pop_front();
        ++buf;
        ++total;
    }
    tos::kern::unbusy();
    return b.slice(0, total);
}

void usart0::clear() {
    tos::int_guard ig;
    reset(state->have_data, 0);
    state->read_buf.clear();
}

span<uint8_t> usart0::read(span<uint8_t> b,
                        tos::alarm<tos::avr::timer1*>& alarm,
                        const std::chrono::milliseconds& to) {
    size_t total = 0;
    auto len = b.size();
    auto buf = b.data();
    tos::kern::busy();
    while (state && total < len) {
        auto res = state->have_data.down(alarm, to);
        if (res == sem_ret::timeout) {
            break;
        }
        *buf = state->read_buf.front();
        state->read_buf.pop_front();
        ++buf;
        ++total;
    }
    tos::kern::unbusy();
    return b.slice(0, total);
}

int usart0::write(span<const uint8_t> buf) {
    write_usart(buf);
    return buf.size() - state->len;
}
} // namespace avr
} // namespace tos

static void put_sync(const uint8_t data) {
    while (!(UCSR0A & (1 << UDRE0)))
        ;
    UDR0 = data;
}

namespace tos {
namespace avr {
void write_sync(span<const uint8_t> buf) {
    for (auto c : buf) {
        put_sync(c);
    }
}
} // namespace avr
} // namespace tos

using tos::avr::state;

ISR(USART_TX_vect) {
    state->write_done.up_isr();
}

ISR(USART_UDRE_vect) {
    if (state->len == 0) {
        UCSR0B &= ~(1 << UDRIE0);
        return;
    }

    UDR0 = *state->write;
    ++state->write;

    state->len--;
}

ISR(USART_RX_vect) {
    auto byte = UDR0;

    state->read_buf.push_back(byte);
    state->have_data.up_isr();
}
