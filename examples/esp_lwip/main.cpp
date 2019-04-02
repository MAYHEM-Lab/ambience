//
// Created by fatih on 7/19/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/lx106/timer.hpp>
#include <arch/lx106/usart.hpp>
#include <arch/lx106/wifi.hpp>
#include <arch/lx106/tcp.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
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

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("WIFI", "PASS");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;


    with (std::move(res), [&](tos::esp82::wifi_connection& conn) {
        conn.wait_for_dhcp();

        auto addr = with(conn.get_addr(), [&](auto& addr){
            return addr;
        }, [](auto){
            return tos::ipv4_addr_t{};
        });

        tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);

        lwip_init();

        tos::esp82::tcp_socket sock(conn, {80});
        if (!sock.is_valid()) {
            tos::println(usart, "nope");
        }

        tos::semaphore s{0};
        tos::tcp_stream<tos::esp82::tcp_endpoint> *ep;

        auto acceptor = [&](auto &, tos::esp82::tcp_endpoint &&newep) {
            if (ep) {
                return false;
            }
            auto mem = os_malloc(sizeof(tos::tcp_stream<tos::esp82::tcp_endpoint>));
            ep = new(mem) tos::tcp_stream<tos::esp82::tcp_endpoint>(std::move(newep));
            s.up();
            return true;
        };

        sock.accept(acceptor);

        int cnt = 0;
        while (true) {
            ep = nullptr;
            tos::println(usart, "waiting", get_count(s));

            s.down();

            tos::println(usart, "hello");

            with(ep->read(buf), [&](auto &req) {
                ++cnt;

                tos::println(*ep, "HTTP/1.0 200 Content-type: text/html");
                tos::println(*ep);
                tos::print(*ep, "<body><b>Hello from Tos!</b><br/><code>");
                tos::print(*ep, req);
                tos::println(*ep, "</code><br/>");
                tos::println(*ep, "<ul>");
                tos::println(*ep, "<li>", tos::platform::board_name, "</li>");
                tos::println(*ep, "<li>", tos::vcs::commit_hash, "</li>");
                tos::println(*ep, "<li>", int(system_get_free_heap_size()), "</li>");
                tos::println(*ep, "<li>", int(system_get_time()), "</li>");
                tos::println(*ep, "<li>", cnt, "</li>");
                tos::println(*ep, "</ul></body>");
                tos::println(*ep);

                tos::println(usart, "wow");
            }, [&](tos::read_error err) {
                tos::println(usart, "disconnected!");
            });

            std::destroy_at(ep);
            os_free(ep);
            ep = nullptr;

            tos::println(usart, "done", cnt, int(system_get_free_heap_size()));
        }
    }, tos::ignore);
}

void tos_main()
{
    tos::launch(tos::alloc_stack, task);
}