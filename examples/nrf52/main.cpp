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

#include <drivers/common/gpio.hpp>
#include <drivers/arch/nrf52/gpio.hpp>
#include <drivers/arch/nrf52/usart.hpp>
#include <algorithm>
#include <tos/print.hpp>

#include <drivers/arch/nrf52/timer.hpp>
#include <drivers/common/alarm.hpp>
#include <drivers/arch/nrf52/radio.hpp>

namespace {
    auto g = tos::open(tos::devs::gpio);

    tos::semaphore sem{0};

    void led1_task() {
        using namespace tos;

        nrf52::timer0 tmr;
        auto alarm = open(devs::alarm, tmr);

        g->write(17, digital::low);
        while (true) {
            g->write(17, digital::high);
            sem.down(alarm, {10000});

            g->write(17, digital::low);
            sem.down(alarm, {10000});
        }
    }

    void led2_task() {
        using namespace tos;
        using namespace tos_literals;
        constexpr auto usconf = tos::usart_config()
                .add(115200_baud_rate)
                .add(usart_parity::disabled)
                .add(usart_stop_bit::one);

        auto usart = open(tos::devs::usart<0>, usconf);

        g->write(19, digital::low);

        nrf52::radio rad;

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
        }
    }
}

void TOS_EXPORT tos_main()
{
    using namespace tos;
    g->set_pin_mode(17, pin_mode::out);
    g->set_pin_mode(19, pin_mode::out);

    tos::launch(led1_task);
    tos::launch(led2_task);
}