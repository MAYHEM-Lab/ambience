//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos_arch.hpp>
#include <tos/print.hpp>
#include <tos/devices.hpp>

ft::semaphore sem(0);

void hello_task()
{
    while (true) {
        sem.down();
        println(*tos::arch::debug_stream(), ft::this_thread::get_id(), ": hello");
    }
}

void yo_task()
{
    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        println(*tos::arch::debug_stream(), ft::this_thread::get_id(), ": yo");
        ft::this_thread::yield();
    }
}

int main()
{
    ft::start(hello_task);
    ft::start(yo_task);

    while (true)
    {
        ft::schedule();
        //using namespace std::chrono_literals;
        //std::this_thread::sleep_for(500ms);
    }
}
