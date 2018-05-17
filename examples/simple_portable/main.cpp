//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>

#include <drivers/common/tty.hpp>

#include <drivers/arch/avr/usart.hpp>
#include <tos/mutex.hpp>
//#include <drivers/arch/x86/stdio.hpp>

tos::semaphore sem(0);

tos::mutex m;
void hello_task() __attribute__((OS_task));
void hello_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    m.lock();
    tos::println(*tty, tos::this_thread::get_id().id, (void*)SP);
    m.unlock();
    while (true) {
        sem.down();
        m.lock();
        tos::println(*tty, tos::this_thread::get_id().id, ": hello");
        m.unlock();
    }
}

void yo_task() __attribute__((OS_task));
void yo_task()
{
    auto tty = tos::open(tos::devs::tty<0>);
    {
        tos::lock_guard<tos::mutex> lock{m};
        tos::println(*tty, tos::this_thread::get_id().id, (void*)SP);
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
