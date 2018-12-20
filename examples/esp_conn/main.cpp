//
// Created by fatih on 7/19/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>

char buf[512];
void task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::println(usart, "\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    tos::esp82::wifi w;
    conn_:
    auto res = w.connect("UCSB Wireless Web", "");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    auto addr = force_get(wconn.get_addr());

    tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);

    lwip_init();

    for (int i = 0; i < 3'000'000; ++i) {
        auto try_conn = tos::esp82::connect(wconn, {{45, 55, 149, 110}}, {80});

        if (!try_conn)
        {
            tos::println(usart, "couldn't connect");
            continue;
        }

        auto& conn = force_get(try_conn);
        tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};

        stream.write("GET / HTTP/1.1\r\n"
                     "Host: bakirbros.com\r\n"
                     "Connection: close\r\n"
                     "\r\n");
        
        while (true)
        {
            auto read_res = stream.read(buf);
            if (!read_res) break;

            auto& r = force_get(read_res);
            tos::print(usart, r);
            tos::this_thread::yield();
        }

        tos::println(usart, "done", i, int(system_get_free_heap_size()));
    }

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = open(tos::devs::alarm, tmr);

    while (true)
    {
        using namespace std::chrono_literals;
        alarm.sleep_for(10s);
        tos::println(usart, "tick!");
    }
}

void tos_main()
{
    tos::launch(task);
}
