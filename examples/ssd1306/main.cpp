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
#include <common/clock.hpp>
#include <tos/gfx/text.hpp>
#include <tos/gfx/dimensions.hpp>

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

constexpr auto font = tos::gfx::basic_font().rotate_90_cw().rotate_90_cw().rotate_90_cw();

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

    ssd1306 oled{&t, {0x3C}, 128, 64};
    oled.dim(false);

    auto tmr = open(devs::timer<2>);
    auto alarm = open(devs::alarm, tmr);

    auto tmr3 = open(devs::timer<3>);
    tos::clock clk(&tmr3);

    std::array<char, 20> buf;
    auto last = clk.now();
    while (true)
    {
        tos::omemory_stream str(buf);
        auto now = clk.now();
        tos::print(str, int((now - last).count()));

        draw_text(oled,
                  font,
                  str.get(),
                  tos::gfx::point{36, 0},
                  tos::gfx::text_direction::vertical);

        last = clk.now();
        oled.display();
    }

    bool go = true;
    uint32_t x = 0;
    while (true) {
        using namespace std::chrono_literals;

        for (int i = 0; i < 128; ++i) {
            oled.set_pixel(x % 64, i, go);
        }
        oled.display();
        if (x % 64 == 63) {
            go ^= true;
        }

        tos::println(usart, "tick", int(x));
        ++x;
    }
}

namespace {
tos::stack_storage<2048> stack;
}
void tos_main() {
    tos::launch(stack, lcd_main);
}