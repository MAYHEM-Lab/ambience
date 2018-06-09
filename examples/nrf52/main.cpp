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

namespace {
    tos::semaphore sem{0};

    auto g = tos::open(tos::devs::gpio);

    void led1_task() {
        using namespace tos;
        g->write(17, digital::low);
        while (true) {
            g->write(17, digital::high);
            for (int i = 0; i < 1000; ++i) {
                nrf_delay_us(100);
                tos::this_thread::yield();
            }

            g->write(17, digital::low);
            for (int i = 0; i < 1000; ++i) {
                nrf_delay_us(100);
                tos::this_thread::yield();
            }
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

        char x;
        while (true) {
            usart.read({&x, 1});
            g->write(19, digital::high);

            usart.read({&x, 1});
            g->write(19, digital::low);
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