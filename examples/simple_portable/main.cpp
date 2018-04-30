//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>

#include <drivers/common/tty.hpp>

//#include <drivers/arch/avr/usart.hpp>
#include <drivers/arch/x86/stdio.hpp>

tos::semaphore sem(0);

void hello_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    while (true) {
        sem.down();
        tos::println(*tty, tos::this_thread::get_id(), ": hello");
    }
}

void yo_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        tos::println(*tty, tos::this_thread::get_id(), ": yo");
        tos::this_thread::yield();
    }
}

int main()
{
    tos::launch(hello_task);
    tos::launch(yo_task);

    while (true)
    {
        tos::schedule();
    }
}
