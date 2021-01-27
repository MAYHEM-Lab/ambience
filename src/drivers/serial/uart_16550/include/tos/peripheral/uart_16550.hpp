#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/self_pointing.hpp>
#include <tos/span.hpp>
#include <tos/x86_64/port.hpp>

namespace tos::x86_64 {
struct uart_16550 : tos::self_pointing<uart_16550> {
public:
    static expected<uart_16550, nullptr_t> open(uint16_t base_port = 0x3F8) {
        auto res = uart_16550();
        res.m_base_port = base_port;
        res.int_en().outb(0);
        res.line_ctrl().outb(0x80); // Enable DLAB, maps 0 and 1 registers to divisor

        res.div_lo().outb(3); // 38400 baud
        res.div_hi().outb(0);

        res.line_ctrl().outb(0x03);  // 8 bits, no parity, one stop bit
        res.fifo_ctrl().outb(0xc7);  // Enable FIFO, clear them, with 14-byte threshold
        res.modem_ctrl().outb(0x0b); // IRQs enabled, RTS/DSR set

        res.modem_ctrl().outb(0x1e);

        res.data().outb(0xf6);

        if (res.data().inb() != 0xf6) {
            return unexpected(nullptr);
        }

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

private:
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
} // namespace tos::x86_64
