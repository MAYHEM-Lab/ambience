//
// Created by fatih on 3/20/18.
//

#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <tos/interrupt.hpp>
#include <tos/ft.hpp>
#include <boost/asio.hpp>

namespace asio = boost::asio;

void tos_main();

namespace boost
{
    void throw_exception(std::exception const& e){
        std::cerr << "exception: " << e.what() << '\n';
        std::abort();
    }
}

static asio::io_service* g_io;
asio::io_service& get_io()
{
    return *g_io;
}

int main()
{
    asio::io_service io;
    g_io = &io;

    tos::kern::enable_interrupts();

    tos_main();
    tos::sched.schedule();
    io.run_one();

    while (true)
    {
        auto res = tos::sched.schedule();
        if (io.stopped())
            io.reset();
        io.run_one();
        if (res == tos::exit_reason::restart)
        {
            return 0;
        }
    }
}