//
// Created by fatih on 12/6/18.
//

#include <arch/drivers.hpp>
#include <common/lcd.hpp>
#include <common/ssd1306.hpp>
#include <tos/ft.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

namespace tos {
void delay_ms(std::chrono::milliseconds ms) {
    delay_us(ms);
}

void delay_us(std::chrono::microseconds us) {
    uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i) {
        __asm__ __volatile__("nop");
    }
}
} // namespace tos

void usart_setup(tos::stm32::gpio& g) {
    using namespace tos::tos_literals;

    auto tx_pin = 2_pin;
    auto rx_pin = 3_pin;

    g.set_pin_mode(rx_pin, tos::pin_mode::in);

    gpio_set_mode(
        GPIOA, GPIO_MODE_OUTPUT_50_MHZ, GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_TX);
}

void lcd_main() {
    using namespace tos::tos_literals;
    using namespace tos;
    using namespace tos::stm32;

    auto g = tos::open(tos::devs::gpio);

    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;
    twim t{24_pin, 25_pin};

    ssd1306<twim*> l{&t, {0x3C}, 128, 64};
    l.dim(false);

    auto tmr = open(devs::timer<2>);
    auto alarm = open(devs::alarm, tmr);

    bool go = true;
    uint32_t x = 0;
    while (true) {
        using namespace std::chrono_literals;

        // tos::println(usart, "tick", x);

        for (int i = 0; i < 128; ++i) {
            l.set_pixel(x % 64, i, go);
        }
        l.display();
        if (x % 64 == 63) {
            go ^= true;
            alarm->sleep_for(5s);
        }

        usart_setup(g);
        auto usart = tos::open(tos::devs::usart<1>, tos::uart::default_9600);

        tos::println(usart, "tick", int(x));
        ++x;

        std::array<char, 1> b;
        usart->read(b, alarm, 10ms);
    }
}

void tos_main() {
    tos::launch(tos::stack_size_t{512}, lcd_main);
}