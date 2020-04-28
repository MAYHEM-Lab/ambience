//
// Created by fatih on 7/19/18.
//

#include <arch/drivers.hpp>
#include <common/inet/tcp_stream.hpp>
#include <lwip/init.h>
#include <tos/devices.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>
#include <tos/utility.hpp>
#include <tos/build.hpp>

uint8_t buf[512];
void task() {
    using namespace tos::tos_literals;
    using namespace std::chrono_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_115200);

    tos::println(usart, "\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::build::commit_hash());

    int rd_loop = 0;
    int success = 0;
    int fail = 0;
    int state = 0;
    int i = 0;
    auto log_ip_task = [&] {
        auto timer = tos::open(tos::devs::timer<0>);
        auto alarm = tos::open(tos::devs::alarm, timer);

        tos::println(usart, "Logger thread running");
        tos::println(usart, "Logger thread:", tos::self()->get_ctx().buf[0]);

        while (true) {
            tos::this_thread::sleep_for(alarm, 1s);

            tos::println(usart, "Main thread:", state, i, success, rd_loop, fail);
        }
    };

    tos::launch(tos::alloc_stack, log_ip_task);

    tos::esp82::wifi w;
conn_:
    auto res = w.connect("mayhem", "z00mz00m");
    state = 1;

    if (!res)
        goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    state = 2;

    for (; true; ++i) {
        auto try_conn =
            tos::esp82::connect(wconn, tos::parse_ipv4_address("93.184.216.34"), {80});
        state = 3;

        if (!try_conn) {
            ++fail;
            continue;
        }

        auto& conn = force_get(try_conn);
        tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};
        state = 4;
        tos::println(stream, "GET / HTTP/1.1");
        tos::println(stream, "Host: example.com");
        tos::println(stream, "Connection: close");
        tos::println(stream);

        state = 6;

        rd_loop = 0;
        while (true) {
            auto read_res = stream.read(buf);
            state = 8;
            if (!read_res)
                break;

            auto& r = force_get(read_res);
            // tos::print(usart, r);
            ++rd_loop;
            if (rd_loop > 10'000)
                break;
            state = 9;
            tos::this_thread::yield();
        }

        state = 7;
        ++success;
    }
}

void tos_main() {
    tos::launch(tos::alloc_stack, task);
}
