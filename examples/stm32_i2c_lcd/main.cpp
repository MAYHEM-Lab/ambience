//
// Created by fatih on 12/6/18.
//

#include <common/lcd.hpp>
#include <tos/ft.hpp>
#include <arch/drivers.hpp>

#include <tos/version.hpp>
#include <tos/mem_stream.hpp>
#include <tos/print.hpp>

namespace tos
{
    void delay_ms(std::chrono::milliseconds ms) {
        delay_us(ms);
    }

    void delay_us(std::chrono::microseconds us)
    {
        uint32_t end = (us.count() * (rcc_ahb_frequency / 1'000'000)) / 13.3;
        for (volatile int i = 0; i < end; ++i)
        {
            __asm__ __volatile__ ("nop");
        }
    }
} // namespace tos

void lcd_main()
{
    using namespace tos::tos_literals;
    using namespace tos; using namespace tos::stm32;

    // need a proper API for this alternate function IO business ...
    rcc_periph_clock_enable(RCC_AFIO);
    AFIO_MAPR |= AFIO_MAPR_I2C1_REMAP;
    twim t { 24_pin, 25_pin };

    lcd<twim> lcd{ t, { 0x27 }, 20, 4 };

    auto tmr = open(devs::timer<2>);
    auto alarm = open(devs::alarm, tmr);

    lcd.begin(alarm);
    lcd.backlight();

    int x = 0;
    while (true)
    {
        ++x;
        using namespace std::chrono_literals;

        lcd.set_cursor(0, 0);
        tos::print(lcd, "Tos on STM32");

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
    tos::launch(tos::alloc_stack, lcd_main);
}