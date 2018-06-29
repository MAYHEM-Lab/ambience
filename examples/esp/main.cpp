//
// Created by fatih on 4/26/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <drivers/common/alarm.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/utility.hpp>
#include <tos/memory.hpp>

#include <drivers/arch/lx106/timer.hpp>
#include <drivers/arch/lx106/usart.hpp>
#include <drivers/arch/lx106/wifi.hpp>
#include <drivers/arch/lx106/tcp.hpp>

extern "C"
{
#include <mem.h>
}

void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, 19200_baud_rate);
    usart->enable();
    tos::print(*usart, "\n\n\n\n\n\n");

    tos::esp82::wifi w;
    auto res = w.connect("FG", "23111994a");

    tos::println(*usart, "connected?", res);

    while (!w.wait_for_dhcp());

    if (res)
    {
        tos::esp82::wifi_connection conn;
        auto addr = conn.get_addr();
        tos::println(*usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
    }

    tos::esp82::tcp_socket sock{ w, { 1000 } };
    tos::fixed_fifo<int, 24> buf;

    auto recv_handler = [&buf](tos::esp82::tcp_endpoint& ep, tos::span<const char> foo){
        buf.push(foo.size());
        ep.send("hello");
    };

    auto sent_handler = [&buf](tos::esp82::tcp_endpoint& ep){
        buf.push(42);
        tos::std::destroy_at(&ep);
        os_free(&ep);
    };

    auto handler = [&](tos::esp82::tcp_socket&, tos::esp82::tcp_endpoint new_ep){
        auto mem = os_malloc(sizeof new_ep);
        auto ep = new (mem) tos::esp82::tcp_endpoint(std::move(new_ep));
        ep->attach(tos::esp82::events::recv, recv_handler);
        ep->attach(tos::esp82::events::sent, sent_handler);
    };

    sock.accept(handler);

    buf.push(100);

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