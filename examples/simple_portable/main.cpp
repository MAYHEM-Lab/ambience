//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/compiler.hpp>

#include <drivers/common/tty.hpp>
#include <drivers/common/usart.hpp>

#ifdef TOS_ARCH_avr
#include <drivers/arch/avr/usart.hpp>
#elif defined(TOS_ARCH_x86)
#include <drivers/arch/x86/stdio.hpp>
#elif defined(TOS_ARCH_arm)
#include <drivers/arch/nrf52/usart.hpp>
#endif

#include <tos/mutex.hpp>

tos::semaphore sem(0);

tos::mutex m;
void TOS_TASK hello_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    {
        tos::lock_guard<tos::mutex> lock{m};
        tos::println(*tty, tos::this_thread::get_id().id);
    }

    while (true) {
        sem.down();
        {
            tos::lock_guard<tos::mutex> lock{m};
            tos::println(*tty, tos::this_thread::get_id().id, ": hello");
        }
    }
}

void TOS_TASK yo_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    {
        tos::lock_guard<tos::mutex> lock{m};
        tos::println(*tty, tos::this_thread::get_id().id);
    }

    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        {
            tos::lock_guard<tos::mutex> lock{m};
            tos::println(*tty, tos::this_thread::get_id().id, ": yo", i);
        }
        tos::this_thread::yield();
    }
}

void tos_main()
{
    using namespace tos;
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::launch(hello_task);
    tos::launch(yo_task);
}
