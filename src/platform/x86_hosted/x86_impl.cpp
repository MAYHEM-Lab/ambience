//
// Created by fatih on 3/20/18.
//

#include <boost/asio.hpp>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <tos/ft.hpp>
#include <tos/interrupt.hpp>

namespace asio = boost::asio;

void tos_main();

namespace boost {
void throw_exception(std::exception const& e) {
    std::cerr << "exception: " << e.what() << '\n';
    std::abort();
}
} // namespace boost

static asio::io_service* g_io;
asio::io_service& get_io() {
    return *g_io;
}

extern "C" {
USED int main() {
    asio::io_service io;
    g_io = &io;

    tos::kern::enable_interrupts();

    tos_main();
    tos::global::sched.schedule();
    io.run_one();

    while (true) {
        auto res = tos::global::sched.schedule();
        if (io.stopped()) {
            io.reset();
        }
        io.poll();
        if (res == tos::exit_reason::restart) {
            break;
        }
    }

    io.run();
    return 0;
}
}