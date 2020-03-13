#include <arch/messagebox.hpp>
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
} // namespace
uart0::uart0() {
    property_channel_tags_builder builder;
    auto buf = builder.add(0x38002, {2, 4000000, 0}).end();
    property_channel property;
    property.transaction(buf);

    // Disable UART0.
    UART0->CR = 0;

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
    UART0->LCRH = 0b11<<5;
    UART0->CR = 0x301;
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
} // namespace tos::raspi3