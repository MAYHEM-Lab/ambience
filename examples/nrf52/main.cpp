//
// Created by Mehmet Fatih BAKIR on 29/03/2018.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>

#include <nrf_delay.h>
#include <nrf_gpio.h>

#include <nrfx_uarte.h>
#include <drivers/include/nrfx_uarte.h>
#include <tos/compiler.hpp>

#include <arch/nrf52/drivers.hpp>
#include <algorithm>
#include <tos/print.hpp>

#include <common/alarm.hpp>
#include <common/lcd.hpp>

namespace {
    auto g = tos::open(tos::devs::gpio);

    tos::semaphore sem{0};

    void led1_task(void*) {
        using namespace tos;

        auto tmr = open(tos::devs::timer<0>);
        auto alarm = open(devs::alarm, tmr);

        g->write(17, digital::low);
        while (true) {
            g->write(17, digital::high);
            using namespace std::chrono_literals;
            sem.down(alarm, 1s);

            g->write(17, digital::low);
            sem.down(alarm, 1s);
        }
    }

    char c;
    tos::semaphore send{0};
    tos::semaphore sent{0};
    void i2c_task(void*)
    {
        using namespace tos;
        nrf52::twim i2c{26, 25};

        lcd<nrf52::twim> lcd{ i2c, { 0x27 }, 20, 4 };

        char buf[] = "Hello World";
        char buf2[] = "Tos (c631797)";

        auto tmr = open(devs::timer<0>);
        auto alarm = open(devs::alarm, tmr);
        lcd.begin(alarm);

        lcd.backlight();
        lcd.write(buf2);
        lcd.set_cursor(0, 2);
        lcd.write(buf);
        lcd.set_cursor(0, 1);
        lcd.write(buf);

        g->write(17, digital::low);
        while (true)
        {
            send.down();
            g->write(17, digital::high);
            lcd.write({&c, 1});
            sent.up();
            g->write(17, digital::low);
        }
    }

    void led2_task(void*) {
        using namespace tos;
        using namespace tos_literals;
        constexpr auto usconf = tos::usart_config()
                .add(115200_baud_rate)
                .add(usart_parity::disabled)
                .add(usart_stop_bit::one);

        auto usart = open(tos::devs::usart<0>, usconf);

        g->write(19, digital::low);

        nrf52::radio rad;

        tos::println(usart, "hello world!");

        char x;
        uint32_t i = 0;
        while (true) {
            usart.read({&x, 1});
            g->write(19, digital::high);
            if (x == 'r')
            {
                for (int j = 0; j < 50; ++j)
                {
                    auto res = rad.receive();
                    tos::println(usart, (int32_t)res);
                    rad.transmit(res);
                }
            }
            else if (x == 't')
            {
                for (int j = 0; j < 50; ++j) {
                    auto s = ++i;
                    rad.transmit(s);
                    //rad.transmit(s);
                    auto r = rad.receive();
                    tos::println(usart, "sent", (int32_t) s, "got", (int32_t) r);
                }
            }
            else if (x == 'a')
            {
                sem.up();
            }
            else if (x == 'i')
            {
                usart.read({&c, 1});
                send.up();
                sent.down();
                tos::println(usart, "sent");
            }
        }
    }
}

void TOS_EXPORT tos_main()
{
    using namespace tos;
    g->set_pin_mode(17, pin_mode::out);
    g->set_pin_mode(19, pin_mode::out);

    tos::launch(led1_task);
    //tos::launch(i2c_task);
    tos::launch(led2_task);
}