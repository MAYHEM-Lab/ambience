//
// Created by Mehmet Fatih BAKIR on 15/04/2018.
//

#include "app.hpp"
#include "tos/function_ref.hpp"

#include <arch/drivers.hpp>
#include <avr/io.h>
#include <common/alarm.hpp>
#include <common/dht22.hpp>
#include <common/gpio.hpp>
#include <stdlib.h>
#include <tos/arch.hpp>
#include <tos/compiler.hpp>
#include <tos/devices.hpp>
#include <tos/event.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>
#include <util/delay.h>


void main_task() {
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    auto g = tos::open(tos::devs::gpio);

    g->set_pin_mode(2_pin, tos::pin_mode::in_pullup);

    tos::event pinsem;
    auto handler = [&] { pinsem.fire(); };

    tos::avr::exti external_interrupts;
    external_interrupts->attach(
        2_pin, tos::pin_change::rising, tos::function_ref<void()>(handler));

    while (true) {
        g.set_pin_mode(13_pin, tos::pin_mode::out);
        g.write(13_pin, tos::digital::high);

        g.set_pin_mode(10_pin, tos::pin_mode::in_pullup);

        auto tmr = open(tos::devs::timer<1>);
        tos::alarm alarm(&*tmr);

        auto d =
            tos::make_dht(g, [](std::chrono::microseconds us) { _delay_us(us.count()); });

        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 2s);

        tos::print(usart, "hi");

        usart.clear();
        std::array<uint8_t, 2> buf;
        auto r = usart.read(buf, alarm, 5s);

        if (r.size() == 2) {
            auto res = d.read(10_pin);

            int retries = 0;
            while (res != tos::dht_res::ok) {
                // tos::println(usart, int8_t(res));
                tos::this_thread::sleep_for(alarm, 2s);
                if (retries == 5) {
                    break;
                    // err
                }
                ++retries;
                res = d.read(10_pin);
            }

            temp::sample s{d.temperature, d.humidity, temp::GetTemp(alarm)};
            struct {
                uint8_t chk_sum{0};
                decltype(usart)* str;
                int write(span<const uint8_t> buf) {
                    for (auto c : buf) {
                        chk_sum += uint8_t(c);
                    }
                    static uint8_t b[13];
                    std::memcpy(b, buf.data(), 12);
                    b[12] = chk_sum;
                    auto res = str->write(b);
                    return res - 1;
                }
            } chk_str;
            chk_str.str = &usart;
            chk_str.write({(const uint8_t*)&s, sizeof s});
            // usart.write({ (const char*)&chk_str.chk_sum, 1 });
        }

        g.write(13_pin, tos::digital::low);

        pinsem.wait();
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, main_task);
}