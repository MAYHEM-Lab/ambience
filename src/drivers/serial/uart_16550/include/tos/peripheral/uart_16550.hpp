#pragma once

#include "tos/fixed_fifo.hpp"
#include "tos/mutex.hpp"
#include "tos/platform.hpp"
#include "tos/x86_64/exception.hpp"
#include "tos/x86_64/pic.hpp"
#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::x86_64 {
struct uart_16550_base {

protected:
    port data() const {
        return {m_base_port};
    }
    port int_en() const {
        return port(m_base_port + 1);
    }

    port div_lo() const {
        return port(m_base_port);
    }

    port div_hi() const {
        return port(m_base_port + 1);
    }

    port fifo_ctrl() const {
        return port(m_base_port + 2);
    }
    port line_ctrl() const {
        return port(m_base_port + 3);
    }
    port modem_ctrl() const {
        return port(m_base_port + 4);
    }
    port line_sts() const {
        return port(m_base_port + 5);
    }

    uint16_t m_base_port;
};
struct uart_16550
    : uart_16550_base
    , tos::self_pointing<uart_16550> {
public:
    static expected<uart_16550, nullptr_t> open(uint16_t base_port = 0x3F8) {
        auto res = uart_16550();
        res.m_base_port = base_port;
        res.int_en().outb(0);
        res.line_ctrl().outb(0x80); // Enable DLAB, maps 0 and 1 registers to divisor

        res.div_lo().outb(3); // 38400 baud
        res.div_hi().outb(0);

        res.line_ctrl().outb(0x03); // 8 bits, no parity, one stop bit

        res.fifo_ctrl().outb(0xc7); // Enable FIFO, clear them, with 14-byte threshold

        res.modem_ctrl().outb(0x0b); // IRQs enabled, RTS/DSR set

        //        res.modem_ctrl().outb(0x1e); // Set in loopback mode
        //
        //        res.data().outb(0xf6);
        //
        //        if (res.data().inb() != 0xf6) {
        //            return unexpected(nullptr);
        //        }

        res.modem_ctrl().outb(0x0f);

        return res;
    }

    void write(uint8_t byte) {
        data().outb(byte);
    }

    int write(span<const uint8_t> data) {
        for (auto c : data) {
            this->data().outb(c);
        }
        return data.size();
    }
};

struct interrupt_uart_16550
    : uart_16550_base
    , tos::self_pointing<interrupt_uart_16550> {

    static interrupt_uart_16550* open(uint16_t base_port = 0x3F8, int irq_line = 4) {
        auto res = new interrupt_uart_16550;
        res->m_base_port = base_port;
        res->m_irq_line = irq_line;

        res->int_en().outb(0);
        res->line_ctrl().outb(0x80); // Enable DLAB, maps 0 and 1 registers to divisor

        res->div_lo().outb(3); // 38400 baud
        res->div_hi().outb(0);

        res->line_ctrl().outb(0x03); // 8 bits, no parity, one stop bit

        res->fifo_ctrl().outb(0xc7); // Enable FIFO, clear them, with 14-byte threshold

        res->modem_ctrl().outb(0b1000); // IRQs enabled, RTS/DSR unset

        ensure(tos::platform::take_irq(irq_line));
        tos::platform::set_irq(irq_line,
                               tos::mem_function_ref<&interrupt_uart_16550::irq>(*res));

        res->int_en().outb(0x1); // Data available interrupt

        //        res->modem_ctrl().outb(0x1e); // Set in loopback mode
        //
        //        res->data().outb(0xf6);
        //
        //        if (res->data().inb() != 0xf6) {
        //            return unexpected(nullptr);
        //        }

        res->modem_ctrl().outb(0x0f);
        tos::x86_64::pic::enable_irq(irq_line);

        return res;
    }

    void irq(tos::x86_64::exception_frame* frame, int) {
        if (line_sts().inb() & 1) {
            rx_buf.push_isr(data().inb());
            m_read_sync.up_isr();
        }
        if (line_sts().inb() & 0x20) {
            // send a byte
        }
    }

    void write(uint8_t byte) {
        data().outb(byte);
    }

    int write(span<const uint8_t> data) {
        for (auto c : data) {
            this->data().outb(c);
        }
        return data.size();
    }

    template<class AlarmT>
    tos::span<uint8_t>
    read(tos::span<uint8_t> b, AlarmT& alarm, std::chrono::milliseconds to) {
        lock_guard lk{m_read_busy};

        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        while (total < len) {
            auto res = m_read_sync.down(alarm, to);
            if (res == sem_ret::timeout) {
                break;
            }
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        return b.slice(0, total);
    }

    span<uint8_t> read(span<uint8_t> b) {
        lock_guard lk{m_read_busy};
        size_t total = 0;
        auto len = b.size();
        auto buf = b.data();
        while (total < len) {
            m_read_sync.down();
            *buf = rx_buf.pop();
            ++buf;
            ++total;
        }
        return b.slice(0, total);
    }

    tos::mutex m_read_busy;
    tos::semaphore m_read_sync{0};

    tos::basic_fixed_fifo<uint8_t, 64, tos::ring_buf> rx_buf;

    int m_irq_line;
};
} // namespace tos::x86_64
