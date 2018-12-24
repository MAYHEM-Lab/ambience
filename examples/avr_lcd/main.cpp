//
// Created by fatih on 12/24/18.
//

#include <common/lcd.hpp>
#include <arch/avr/drivers.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>

void lcd_main(void*)
{
    using namespace tos::tos_literals;
    using namespace tos; using namespace tos::avr;

    twim t { 19_pin, 18_pin };

    lcd<twim> lcd{ t, { 0x27 }, 20, 4 };

    auto tmr = open(devs::timer<1>);
    auto alarm = open(devs::alarm, tmr);

    lcd.begin(alarm);
    lcd.backlight();

    int x = 0;
    while (true)
    {
        ++x;
        using namespace std::chrono_literals;

        lcd.set_cursor(0, 0);
        tos::print(lcd, "Tos");

        lcd.set_cursor(0, 1);
        tos::print(lcd, tos::platform::arch_name, tos::platform::vendor_name);

        lcd.set_cursor(0, 2);
        tos::print(lcd, tos::span<const char>(tos::vcs::commit_hash).slice(0, 7));

        lcd.set_cursor(0, 3);
        tos::print(lcd, x);

        alarm.sleep_for(100ms);
    }
}

void tos_main()
{
    tos::launch(lcd_main);
}