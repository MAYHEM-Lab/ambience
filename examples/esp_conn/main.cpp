//
// Created by fatih on 7/19/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>
#include <tos/memory.hpp>

#include <arch/lx106/timer.hpp>
#include <arch/lx106/usart.hpp>
#include <arch/lx106/wifi.hpp>
#include <arch/lx106/tcp.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos_arch.hpp>

#include <lwip/init.h>
#include <tos/algorithm.hpp>
#include <algorithm>
#include <common/inet/tcp_stream.hpp>

extern "C"
{
#include <mem.h>
}

char buf[512];
void task(void*)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);
    usart.enable();

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("WIFI", "PASS");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn){
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        lwip_init();

        for (int i = 0; i < 15'000; ++i) {
            with(tos::esp82::connect(conn, {{45, 55, 149, 110}}, {80}), [&](tos::esp82::tcp_endpoint &conn) {
                tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};

                stream.write("GET / HTTP/1.1\r\n"
                             "Host: bakirbros.com\r\n"
                             "Connection: close\r\n"
                             "\r\n");

                tos::println(stream);

                while (true)
                {
                    auto res = stream.read(buf);
                    if (!res) break;
                    with(std::move(res), [&](tos::span<const char> r){
                        tos::print(usart, r);
                    }, tos::ignore);
                    tos::this_thread::yield();
                }
            }, [&](auto &err) {
                tos::println(usart, "couldn't connect");
            });
            tos::println(usart, "done", i, int(system_get_free_heap_size()));
        }

    }, [&](auto& err){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    auto tmr = open(tos::devs::timer<0>);
    auto alarm = open(tos::devs::alarm, tmr);

    while (true)
    {
        alarm.sleep_for({ 10'000 });
        tos::println(usart, "tick!");
    }
}

void tos_main()
{
    tos::launch(task);
}