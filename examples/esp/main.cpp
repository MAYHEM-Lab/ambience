//
// Created by fatih on 4/26/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>
#include <tos/memory.hpp>
#include <common/inet/tcp_stream.hpp>

#include <arch/lx106/timer.hpp>
#include <arch/lx106/usart.hpp>
#include <arch/lx106/wifi.hpp>
#include <arch/lx106/tcp.hpp>
#include <tos/version.hpp>

extern "C"
{
#include <mem.h>
}

int reqs = 0;
volatile bool running = false;
alignas(alignof(tos::tcp_stream)) char mem[sizeof(tos::tcp_stream)];
tos::tcp_stream* socket;
char buf[512];
char sock_stack[1024];
tos::fixed_fifo<int, 8>* dbg;
void socket_task()
{
    auto req = socket->read(buf);
    tos::println(*socket, "HTTP/1.0 200 Content-type: text/html");
    tos::println(*socket);
    tos::print(*socket, "<body><b>Hello from Tos!</b><br/><code>");
    tos::print(*socket, req);
    tos::println(*socket, "</code><br/>");
    tos::println(*socket, "<ul>");
    tos::println(*socket, "<li>", tos::platform::board_name, "</li>");
    tos::println(*socket, "<li>", tos::vcs::commit_hash, "</li>");
    tos::println(*socket, "<li>", system_get_free_heap_size(), "</li>");
    tos::println(*socket, "<li>", system_get_time(), "</li>");
    tos::println(*socket, "<li>", ++reqs, "</li>");
    tos::println(*socket, "</ul></body>");
    tos::println(*socket);

    tos::std::destroy_at(socket);
    running = false;
}

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

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 80 } };
    tos::fixed_fifo<int, 8> f;
    dbg = &f;

    auto handler = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint new_ep){
        if (running){
            f.push(50);
            return;
        }
        running = true;

        f.push(40);

        socket = new (mem) tos::tcp_stream(std::move(new_ep));

        constexpr auto params = tos::thread_params()
                .add<tos::tags::entry_pt_t>(&socket_task)
                .add<tos::tags::stack_ptr_t>((void*)sock_stack)
                .add<tos::tags::stack_sz_t>(1024U);

        tos::launch(params);
    };

    sock.accept(handler);

    while (true)
    {
        auto x = f.pop();
        tos::println(usart, x);
    }

    tos::semaphore s{0};
    s.down(); // block forever
}

void tos_main()
{
    tos::launch(task);
}