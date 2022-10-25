//
// Created by fatih on 3/20/18.
//

#include <utility> // work around boost missing include bug

#include <boost/asio.hpp>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <tos/interrupt.hpp>
#include <tos/scheduler.hpp>

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
    tos::global::sched.schedule(tos::int_guard{});
    io.run_one();

    while (true) {
        auto res = tos::global::sched.schedule(tos::int_guard{});
        if (res == tos::exit_reason::restart) {
            break;
        }
        if (io.stopped()) {
            io.restart();
        }
        if (res == tos::exit_reason::idle || res == tos::exit_reason::power_down) {
            io.run_one();
        }
        io.poll();
    }

    return 0;
}
}
