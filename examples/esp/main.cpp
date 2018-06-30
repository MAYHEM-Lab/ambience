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

extern "C"
{
#include <mem.h>
}

tos::tcp_stream* socket;
char buf[512];
void socket_task()
{
    auto req = socket->read(buf);
    tos::println(*socket, "HTTP/1.0 200 Content-type: text/html");
    tos::println(*socket);
    tos::print(*socket, "<body><b>Hello from Tos!</b><br/><code>");
    tos::print(*socket, req);
    tos::println(*socket, "</code></body>");
    tos::println(*socket);

    tos::std::destroy_at(socket);
    os_free(socket);
}

void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    tos::print(*usart, "\n\n\n\n\n\n");

    tos::esp82::wifi w;
    auto res = w.connect("WIFI", "PASS");

    tos::println(*usart, "connected?", res);

    while (!w.wait_for_dhcp());

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(*usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 80 } };
    tos::fixed_fifo<int, 24> buf;

    auto handler = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint new_ep){
        auto mem = os_malloc(sizeof(tos::tcp_stream));
        socket = new (mem) tos::tcp_stream(std::move(new_ep));
        tos::launch(socket_task);
    };

    sock.accept(handler);

    buf.push(100);
    tos::println(*usart, tos::platform::board_name);

    while (true)
    {
        auto c = buf.pop();
        tos::println(*usart, c);
    }
}

void tos_main()
{
    tos::launch(task);
}