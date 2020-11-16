#include <arch/mailbox.hpp>
#include <arch/usart.hpp>
#include <tos/soc/bcm2837.hpp>

using namespace tos::bcm2837;
namespace tos::raspi3 {
namespace {
void delay(int32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                 : "=r"(count)
                 : [count] "0"(count)
                 : "cc");
}
} // namespace

uart0::uart0(usart_constraint&&, interrupt_controller& ic)
    : m_handler(tos::mem_function_ref<&uart0::irq>(*this)) {
    // Disable UART0.
    UART0->CR = 0;

    property_channel_tags_builder builder;
    auto buf = builder.add(0x38002, {2, 4'000'000, 0}).end();
    property_channel property;
    if (!property.transaction(buf)) {
        tos::debug::panic("can't set clock speed");
    }

    auto r = GPIO->GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // gpio14, gpio15
    r |= (4 << 12) | (4 << 15);    // alt0
    GPIO->GPFSEL1 = r;

    // Setup the GPIO pin 14 && 15.
    // Disable pull up/down for all GPIO pins & delay for 150 cycles.
    GPIO->GPPUD = 0;
    delay(150);

    // Disable pull up/down for pin 14,15 & delay for 150 cycles.
    GPIO->GPPUDCLK0 = (1 << 14) | (1 << 15);
    delay(150);

    // Write 0 to GPPUDCLK0 to make it take effect.
    GPIO->GPPUDCLK0 = 0x00000000;

    // Clear pending interrupts.
    UART0->ICR = 0x7FF;

    UART0->IBRD = 2;
    UART0->FBRD = 0xB;
    UART0->LCRH = 0b11 << 5;
    UART0->CR = 0x301;

    ic.register_handler(bcm283x::irq_channels::uart, m_handler);
    INTERRUPT_CONTROLLER->enable_irq_2 = 1 << (57 - 32);
    UART0->IMSC = 1 << 4;
}

int uart0::sync_write(tos::span<const uint8_t> buf) {
    for (auto c : buf) {
        while (UART0->FR & (1 << 5))
            ;
        UART0->DR = c;
    }
    while (UART0->FR & (1 << 3))
        ;
    return buf.size();
}

int uart0::write(tos::span<const uint8_t> buf) {
    lock_guard lg{m_lock};
    while (UART0->FR & (1 << 3))
        ;
    {
        int_guard ig;
        m_sendbuf = buf.pop_front();
        UART0->IMSC |= 1 << 5;
        UART0->DR = buf.front();
    }

    m_sem.down();
    return buf.size();
}

span<uint8_t> uart0::read(tos::span<uint8_t> b) {
    size_t total = 0;
    auto len = b.size();
    auto buf = b.data();
    while (total < len) {
        m_recv_sem.down();
        tos::int_guard ig;
        *buf = m_recv_buf.pop();
        ++buf;
        ++total;
    }
    return b.slice(0, total);
}

bool uart0::irq() {
    ++m_stats.irq_count;
    auto mis = UART0->MIS;
    UART0->ICR = mis;

    if (mis & (1 << 4)) {
        ++m_stats.recv_irq_count;

        auto c = UART0->DR;
        tos::debug::do_not_optimize(c);
        if (m_recv_buf.size() < m_recv_buf.capacity()) {
            m_recv_buf.push_isr(c);
            m_recv_sem.up_isr();
        } else {
            ++m_stats.overrun_count;
        }
    }

    if (mis & (1 << 5)) {
        ++m_stats.send_irq_count;

        if (m_sendbuf.empty()) {
            UART0->IMSC &= ~(1 << 5);
            m_sem.up_isr();
        } else {
            UART0->DR = m_sendbuf.front();
            m_sendbuf = m_sendbuf.pop_front();
        }
    }

    return true;
}
} // namespace tos::raspi3