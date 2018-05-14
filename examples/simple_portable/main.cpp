//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>

#include <drivers/common/tty.hpp>

#include <drivers/arch/avr/usart.hpp>
//#include <drivers/arch/x86/stdio.hpp>

tos::semaphore sem(0);

void hello_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    while (true) {
        sem.down();
        tos::println(*tty, tos::this_thread::get_id().id, ": hello");
    }
}

template <class> class p;

void yo_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        tos::println(*tty, tos::this_thread::get_id().id, ": yo");
        tos::this_thread::yield();
    }
}

int main()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->options(
            tos::usart_modes::async,
            tos::usart_parity::disabled,
            tos::usart_stop_bit::one);
    usart->enable();

    tos::enable_interrupts();

    tos::launch(hello_task);
    tos::launch(yo_task);

    while (true)
    {
        tos::schedule();
    }
}
