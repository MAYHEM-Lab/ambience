//
// Created by fatih on 12/24/18.
//

#include <arch/drivers.hpp>
#include <common/lcd.hpp>
#include <tos/build.hpp>
#include <tos/print.hpp>

void lcd_main() {
    using namespace tos::tos_literals;
    using namespace tos;

    auto i2c = tos::open(tos::devs::i2c<0>, tos::i2c_type::master, 19_pin, 18_pin);

    lcd<decltype(&i2c)> lcd{&i2c, {0x27}, 20, 4};

    auto tmr = open(devs::timer<1>);
    auto alarm = open(devs::alarm, tmr);

    lcd.begin(alarm);
    lcd.backlight();

    int64_t x = 0;
    while (true) {
        ++x;
        using namespace std::chrono_literals;

        lcd.set_cursor(0, 0);
        tos::print(lcd, "Tos");

        lcd.set_cursor(0, 1);
        tos::print(lcd, tos::platform::arch_name, tos::platform::vendor_name);

        lcd.set_cursor(0, 2);
        tos::print(lcd,
                   tos::span<const char>(raw_cast<const char>(tos::build::commit_hash()))
                       .slice(0, 7));

        lcd.set_cursor(0, 3);
        tos::print(lcd, int32_t(x));

        tos::this_thread::sleep_for(alarm, 1s);
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, lcd_main);
}