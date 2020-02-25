#include <stddef.h>
#include <stdint.h>

// Loop <delay> times in a way that the compiler won't optimize away
void delay(int32_t count) {
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                         : "=r"(count)
                         : [ count ] "0"(count)
                         : "cc");
}

constexpr auto IO_BASE = 0x3F000000;

constexpr auto GPU_IO_BASE = 0x7E000000;
constexpr auto GPU_MEM_BASE = 0xC0000000;

// Convert ARM address to GPU bus address
constexpr auto gpu_bus_address(uintptr_t addr) {
    return ((addr) & ~0xC0000000) | GPU_MEM_BASE;
}

struct uart0_control_block {
    uint32_t DR;
    uint32_t RSRECR;
    uint8_t __pad__[16];
    uint32_t FR;
    uint8_t __pad_[4];
    uint32_t ILPR;
    uint32_t IBRD;
    uint32_t FBRD;
    uint32_t LCRH;
    uint32_t CR;
    uint32_t IFLS;
    uint32_t IMSC;
    uint32_t RIS;
    uint32_t MIS;
    uint32_t ICR;
    uint32_t DMACR;
    uint32_t ITCR;
    uint32_t ITIP;
    uint32_t ITOP;
    uint32_t TDR;
};

constexpr auto UART0_OFFSET = 0x201000;
constexpr auto UART0_ADDRESS = IO_BASE + UART0_OFFSET;

auto UART0 = reinterpret_cast<volatile uart0_control_block*>(UART0_ADDRESS);

struct gpio_control_block {
    uint8_t __pad__[148];
    uint32_t GPPUD;
    uint32_t GPPUDCLK0;
};

constexpr auto GPIO_OFFSET = 0x200000;
constexpr auto GPIO_ADDRESS = IO_BASE + GPIO_OFFSET;

auto GPIO = reinterpret_cast<volatile gpio_control_block*>(GPIO_ADDRESS);

void uart_init() {
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
    UART0->IMSC = 
               (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8) |
                   (1 << 9) | (1 << 10);

    // Enable UART0, receive & transfer part of UART.
    UART0->CR = (1 << 0) | (1 << 8) | (1 << 9);
}

void uart_putc(unsigned char c) {
    // Wait for UART to become ready to transmit.
    while (UART0->FR & (1 << 5)) {
    }
    UART0->DR = c;
}

unsigned char uart_getc() {
    // Wait for UART to have received something.
    while (UART0->FR & (1 << 4)) {
    }
    return UART0->DR;
}

void uart_puts(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++) uart_putc((unsigned char)str[i]);
}

#if defined(__cplusplus)
extern "C" /* Use C linkage for kernel_main. */
#endif
    void
    kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    // Declare as unused
    (void)r0;
    (void)r1;
    (void)atags;

    uart_init();
    uart_puts("Hello, kernel World!\r\n");

    while (1) {
        auto c = uart_getc();
        uart_putc(c);
    }
}