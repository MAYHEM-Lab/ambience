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
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos_arch.hpp>

#include <lwip/tcp.h>
#include <lwip/init.h>
#include <lwip/timers.h>
#include <tos/algorithm.hpp>
#include <algorithm>
#include <arch/lx106/tcp.hpp>

extern "C"
{
#include <mem.h>
#include <user_interface.h>
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
    auto res = w.connect("Nakedsense.2", "serdar1988");

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

    struct t
    {
        tos::semaphore s{0};
        tos::esp82::tcp_endpoint* ep;
        char buf[16];
        int rlen = 0;
        bool read = false;
        bool sent = false;
    } data;
    int cnt = 0;

    auto handlerecv = [&](auto, auto&, tos::span<const char> recvd){
        data.s.up();
        std::fill(data.buf, data.buf + 16, 0);
        memcpy(data.buf, recvd.data(), tos::std::min<int>(16, recvd.size()));
        data.rlen = recvd.size();
        data.read = true;
    };

    auto handlesent = [&](auto, auto&){
        data.s.up();
        data.sent = true;
    };

    auto acceptor = [&](auto&, tos::esp82::tcp_endpoint&& newep){
        if (data.ep)
        {
            return false;
        }
        data.s.up();
        auto mem = os_malloc(sizeof newep);
        newep.attach(tos::esp82::events::sent, handlesent);
        newep.attach(tos::esp82::events::recv, handlerecv);
        data.ep = new (mem) tos::esp82::tcp_endpoint(std::move(newep));
        return true;
    };

    sock.accept(acceptor);

cnn:
    data.ep = nullptr;
    data.rlen = 0;
    data.read = false;
    data.sent = false;
    tos::println(usart, "waiting", get_count(data.s));

    data.s.down();
    tos::println(usart, "hello");

    recv:
    data.s.down();

    tos::println(usart, "upped");

    if (!data.read)
    {
        if (data.rlen == 0)
        {
            goto cnn;
        }
        goto recv;
    }

    tos::println(usart, "read", data.rlen, data.buf);

    data.ep->send("hello\r\n\r\n");

    send:
    data.s.down();
    if (!data.sent)
    {
        tos::println(usart, "nope");
        goto send;
    }

    tos::println(usart, "wow");

    tos::std::destroy_at(data.ep);
    os_free(data.ep);
    data.ep = nullptr;

    tos::println(usart, "done", ++cnt, int(system_get_free_heap_size()));

    goto cnn;
}

extern "C"
{
uint32 ICACHE_FLASH_ATTR espconn_init(uint32)
{
    return 1;
}
}

void tos_main()
{
    tos::launch(task);
}