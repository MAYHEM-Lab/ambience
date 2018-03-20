//
// Created by fatih on 3/20/18.
//

#include <tos/ft.hpp>
#include <iostream>
#include <thread>
#include <csetjmp>
#include <tos/semaphore.hpp>

ft::semaphore sem(0);

void hello_task()
{
    while (true) {
        sem.down();
        std::cout << ft::this_thread::get_id() << ": hello\n";
    }
}

void yo_task()
{
    for (int i = 0; i < 100; ++i)
    {
        sem.up();
        std::cout << ft::this_thread::get_id() << ": yo\n";
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
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(500ms);
    }
}
