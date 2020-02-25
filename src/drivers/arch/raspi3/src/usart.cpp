#include <arch/usart.hpp>

using namespace bcm2837;
namespace tos::raspi3 {
namespace {
void delay(int32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
    : "=r"(count)
    : [ count ] "0"(count)
    : "cc");
}
}
uart0::uart0() {
    // Disable UART0.
    UART0->CR = 0;

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

    // Set integer & fractional part of baud rate.
    // Divider = UART_CLOCK/(16 * Baud)
    // Fraction part register = (Fractional part * 64) + 0.5
    // UART_CLOCK = 3000000; Baud = 115200.

    // Divider = 3000000 / (16 * 115200) = 1.627 = ~1.
    UART0->IBRD = 1;
    // Fractional part register = (.627 * 64) + 0.5 = 40.6 = ~40.
    UART0->FBRD = 40;

    // Enable FIFO & 8 bit data transmission (1 stop bit, no parity).
    UART0->LCRH = (1 << 4) | (1 << 5) | (1 << 6);

    // Mask all interrupts.
    UART0->IMSC = (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) |
                  (1 << 9) | (1 << 10);

    // Enable UART0, receive & transfer part of UART.
    UART0->CR = (1 << 0) | (1 << 8) | (1 << 9);
}

int uart0::write(tos::span<const uint8_t> buf) {
    for (auto c : buf) {
        while (UART0->FR & (1 << 5)) {
        }
        UART0->DR = c;
    }
    return buf.size();
}
span<uint8_t> uart0::read(tos::span<uint8_t> buf) {
    for (auto& c : buf) {
        while (UART0->FR & (1 << 4)) {
        }
        c = UART0->DR;
    }
    return buf;
}
}