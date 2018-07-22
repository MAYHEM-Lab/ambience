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
void task()
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

    tos::println(usart, "connected?", res);
    if (!res) goto conn;

    while (!w.wait_for_dhcp());

    tos::esp82::wifi_connection conn;
    auto addr = conn.get_addr();
    tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);

    lwip_init();

    tos::esp82::tcp_socket sock(w, {80});
    if (!sock.is_valid())
    {
        tos::println(usart, "nope");
    }

    tos::semaphore s{0};
    tos::tcp_stream* ep;

    auto acceptor = [&](auto&, tos::esp82::tcp_endpoint&& newep){
        if (ep)
        {
            return false;
        }
        auto mem = os_malloc(sizeof(tos::tcp_stream));
        ep = new (mem) tos::tcp_stream(std::move(newep));
        s.up();
        return true;
    };

    sock.accept(acceptor);

    int cnt = 0;
    while (true)
    {
        ep = nullptr;
        tos::println(usart, "waiting", get_count(s));
        s.down();
        ++cnt;

        tos::println(usart, "hello");

        auto req = ep->read({buf, 64});

        auto socket = ep;
        tos::println(*socket, "HTTP/1.0 200 Content-type: text/html");
        tos::println(*socket);
        tos::print(*socket, "<body><b>Hello from Tos!</b><br/><code>");
        tos::print(*socket, req);
        tos::println(*socket, "</code><br/>");
        tos::println(*socket, "<ul>");
        tos::println(*socket, "<li>", tos::platform::board_name, "</li>");
        tos::println(*socket, "<li>", tos::vcs::commit_hash, "</li>");
        tos::println(*socket, "<li>", int(system_get_free_heap_size()), "</li>");
        tos::println(*socket, "<li>", int(system_get_time()), "</li>");
        tos::println(*socket, "<li>", cnt, "</li>");
        tos::println(*socket, "</ul></body>");
        tos::println(*socket);

        tos::println(usart, "wow");

        tos::std::destroy_at(ep);
        os_free(ep);
        ep = nullptr;

        tos::println(usart, "done", cnt, int(system_get_free_heap_size()));
    }
}

void tos_main()
{
    tos::launch(task);
}