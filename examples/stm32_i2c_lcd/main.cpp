//
// Created by fatih on 12/6/18.
//

#include <arch/drivers.hpp>
#include <common/lcd.hpp>
#include <tos/ft.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

namespace tos {
void delay_ms(std::chrono::milliseconds ms) {
    HAL_Delay(ms.count());
}

void delay_us(std::chrono::microseconds us) {
    HAL_Delay(1);
}
} // namespace tos

void lcd_main() {
    using namespace tos::tos_literals;
    using namespace tos;

    tos::stm32::i2c t{tos::stm32::detail::i2cs[0], 24_pin, 25_pin};

    lcd lcd{&t, {0x27}, 20, 4};

    auto tmr = open(devs::timer<2>);
    tos::alarm alarm(&tmr);

    lcd.begin(alarm);
    lcd.backlight();

    int x = 0;
    while (true) {
        ++x;
        using namespace std::chrono_literals;

        lcd.set_cursor(0, 0);
        tos::print(lcd, "Tos on STM32");

        lcd.set_cursor(0, 3);
        tos::print(lcd, x);

        tos::this_thread::sleep_for(alarm, 100ms);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, lcd_main);
}