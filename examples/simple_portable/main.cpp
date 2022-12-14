//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/compiler.hpp>

#include <common/tty.hpp>
#include <common/usart.hpp>
#include <tos/char_stream.hpp>

#include <arch/drivers.hpp>

#include <tos/mutex.hpp>

tos::semaphore sem(0);

tos::char_ostream* out_str;

tos::mutex m;
void TOS_TASK hello_task()
{
    {
        tos::lock_guard<tos::mutex> lock{m};
        tos::println(*out_str, tos::this_thread::get_id().id);
    }

    while (true) {
        sem.down();
        {
            tos::lock_guard<tos::mutex> lock{m};
            tos::println(*out_str, tos::this_thread::get_id().id, ": hello");
        }
    }
}

void TOS_TASK yo_task()
{
    {
        tos::lock_guard<tos::mutex> lock{m};
        tos::println(*out_str, tos::this_thread::get_id().id);
    }

    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        {
            tos::lock_guard<tos::mutex> lock{m};
            tos::println(*out_str, tos::this_thread::get_id().id, ": yo", i);
        }
        tos::this_thread::yield();
    }
}

void tos_main()
{
    using namespace tos;
    using namespace tos::tos_literals;

#if !defined(TOS_PLATFORM_x86)
    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(usart_parity::disabled)
            .add(usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    auto sf = new stream_facade<decltype(usart)>(std::move(usart));
    out_str = sf;
#endif

    tos::launch(tos::alloc_stack, hello_task);
    tos::launch(tos::alloc_stack, yo_task);
}
