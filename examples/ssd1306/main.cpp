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
    uint32_t end = (us.count() * (tos::stm32::ahb_clock / 1'000'000)) / 13.3;
    for (volatile int i = 0; i < end; ++i) {
        __asm__ __volatile__("nop");
    }
}
} // namespace tos

void lcd_main() {
    using namespace tos::tos_literals;
    using namespace tos;
    using namespace tos::stm32;

    auto g = tos::open(tos::devs::gpio);

    auto usart_rx_pin = 3_pin;
    auto usart_tx_pin = 2_pin;

    auto usart =
        open(devs::usart<2>, tos::uart::default_9600, usart_rx_pin, usart_tx_pin);
    tos::println(usart, "hello");

    i2c t{ stm32::detail::i2cs[0], 24_pin, 25_pin };
    tos::println(usart, "i2c init'd");

    ssd1306 l{&t, {0x3C}, 128, 64};
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

        tos::println(usart, "tick", int(x));
        ++x;

        std::array<char, 1> b;
        usart->read(b, alarm, 10ms);
    }
}

namespace {
tos::stack_storage<2048> stack;
}
void tos_main() {
    tos::launch(stack, lcd_main);
}