//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include <arch/avr/drivers.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/arch.hpp>
#include <common/gpio.hpp>
#include <tos/devices.hpp>
#include <stdlib.h>
#include <common/dht22.hpp>
#include <util/delay.h>
#include <tos/event.hpp>
#include <common/alarm.hpp>
#include <avr/io.h>
#include <tos/compiler.hpp>
#include <tos/semaphore.hpp>
#include "app.hpp"


void main_task()
{
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    auto g = tos::open(tos::devs::gpio);

    g->set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event pinsem;
    auto handler = [&]{
        pinsem.fire();
    };

    g->attach_interrupt(2_pin, tos::pin_change::rising, handler);

    while (true)
    {
        g.set_pin_mode(13_pin, tos::pin_mode::out);
        g.write(13_pin, tos::digital::high);

        g.set_pin_mode(10_pin, tos::pin_mode::in_pullup);

        auto tmr = open(tos::devs::timer<1>);
        auto alarm = open(tos::devs::alarm, *tmr);

        auto d = tos::make_dht(g, [](std::chrono::microseconds us) {
            _delay_us(us.count());
        });

        using namespace std::chrono_literals;
        alarm.sleep_for(2s);

        tos::print(usart, "hi");

        usart.clear();
        std::array<char, 2> buf;
        auto r = usart.read(buf, alarm, 5s);

        if (r.size() == 2)
        {
            auto res = d.read(10_pin);

            int retries = 0;
            while (res != tos::dht_res::ok)
            {
                //tos::println(usart, int8_t(res));
                alarm.sleep_for(2s);
                if (retries == 5)
                {
                    break;
                    // err
                }
                ++retries;
                res = d.read(10_pin);
            }

            temp::sample s { d.temperature, d.humidity, temp::GetTemp(alarm) };
            struct
            {
                uint8_t chk_sum{ 0 };
                decltype(usart)* str;
                int write(span<const char> buf)
                {
                    for (auto c : buf)
                    {
                        chk_sum += uint8_t(c);
                    }
                    static char b[13];
                    std::memcpy(b, buf.data(), 12);
                    b[12] = chk_sum;
                    auto res = str->write(b);
                    return res - 1;
                }
            } chk_str;
            chk_str.str = &usart;
            chk_str.write({ (const char*)&s, sizeof s });
            //usart.write({ (const char*)&chk_str.chk_sum, 1 });
        }

        g.write(13_pin, tos::digital::low);

        pinsem.wait();
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, main_task);
}