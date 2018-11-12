//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <drivers/arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <drivers/common/gpio.hpp>
#include <tos/devices.hpp>
#include <stdlib.h>
#include <drivers/common/dht22.hpp>
#include <util/delay.h>
#include <tos/event.hpp>
#include <drivers/common/alarm.hpp>
#include <avr/io.h>
#include <tos/compiler.hpp>
#include <ft/include/tos/semaphore.hpp>
#include "app.hpp"


void main_task(void*)
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(9600_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto g = tos::open(tos::devs::gpio);

    g.set_pin_mode(10_pin, tos::pin_mode::in_pullup);

    auto tmr = open(tos::devs::timer<1>);
    auto alarm = open(tos::devs::alarm, *tmr);

    auto d = tos::make_dht(g, [](std::chrono::microseconds us) {
        _delay_us(us.count());
    });

    using namespace std::chrono_literals;
    alarm.sleep_for(2s);

    tos::print(usart, "hi");

    std::array<char, 2> buf;
    usart.read(buf);

    auto res = d.read(10_pin);

    int retries = 0;
    while (res != tos::dht_res::ok)
    {
        //tos::println(usart, int8_t(res));
        alarm.sleep_for(2s);
        if (retries == 5)
        {
            // err
        }
        ++retries;
        res = d.read(10_pin);
    }

    if (buf[1] == 'o')
    {
        temp::sample s { d.temperature, d.humidity, temp::GetTemp(alarm) };
        struct
        {
            uint8_t chk_sum{ 0 };
            decltype(usart)* str;
            int write(span<const char> buf)
            {
                auto res = str->write(buf);
                for (auto c : buf)
                {
                    chk_sum += uint8_t(c);
                }
                return res;
            }
        } chk_str;
        chk_str.str = &usart;
        chk_str.write({ (const char*)&s, sizeof s });
        usart.write({ (const char*)&chk_str.chk_sum, 1 });
    }
    else
    {
        static char b[32];
        tos::println(usart, int8_t(res));
        tos::println(usart, "Temperature:", dtostrf(d.temperature, 2, 2, b));
        tos::println(usart, "Humidity:", dtostrf(d.humidity, 2, 2, b));
    }

    tos::this_thread::block_forever();
    //tos::semaphore{0}.down();
    //auto st = GetTemp(alarm);
    //tos::println(usart, "Internal:", dtostrf(st, 2, 2, b));
}

void tos_main()
{
    tos::launch(main_task);
}