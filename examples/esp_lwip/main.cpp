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
    auto pcb = tcp_new();
    ip_addr a;
    memcpy(&a.addr, addr.addr, 4);
    auto err = tcp_bind(pcb, IP_ADDR_ANY, 80);

    if (err != ERR_OK) {
        tos::println(usart, "bind failed!");
        tcp_close(pcb);
        return;
    }

    auto listen_pcb = tcp_listen(pcb);
    if (!listen_pcb) {
        tos::println(usart, "listen failed!");
        tcp_close(pcb);
        return;
    }

    struct t
    {
        tos::semaphore s{0};
        tcp_pcb* cl;
        char buf[16];
        int rlen = 0;
        bool read = false;
        bool sent = false;
    } data;
    int cnt = 0;

    static auto recvhandle = [](void* user, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) -> err_t {
        auto& sem = *static_cast<t*>(user);
        sem.s.up();
        sem.rlen = 0;
        sem.read = false;
        std::fill(sem.buf, sem.buf + 16, 0);
        if (p != nullptr)
        {
            memcpy(sem.buf, p->payload, tos::std::min<int>(16, p->len));
            sem.rlen = p->len;
            sem.read = true;
            pbuf_free(p);
        }
        system_os_post(tos::esp82::main_task_prio, 0, 0);
        return ERR_OK;
    };

    static auto senthandle = [](void* user, struct tcp_pcb *tpcb, u16_t len) -> err_t {
        auto& sem = *static_cast<t*>(user);
        sem.s.up();
        sem.sent = true;
        system_os_post(tos::esp82::main_task_prio, 0, 0);
        return ERR_OK;
    };

    tcp_arg(listen_pcb, &data);
    tcp_accept(listen_pcb, [](void* user, tcp_pcb* newpcb, err_t err) -> err_t {
        if (err != ERR_OK)
        {
            return ERR_OK;
        }
        auto& sem = *static_cast<t*>(user);
        if (sem.cl)
        {
            tcp_abort(newpcb);
            return ERR_ABRT;
        }
        sem.s.up();
        sem.cl = newpcb;
        tcp_recv(newpcb, recvhandle);
        tcp_sent(newpcb, senthandle);
        //tcp_nagle_disable(newpcb);
        system_os_post(tos::esp82::main_task_prio, 0, 0);
        return ERR_OK;
    });
cnn:
    data.cl = nullptr;
    data.rlen = 0;
    data.read = false;
    data.sent = false;
    tos::println(usart, "waiting", get_count(data.s));

    data.s.down();
    tos::println(usart, "hello");

    tcp_arg(data.cl, &data);

    /*tcp_poll(data.cl, [](void *user, struct tcp_pcb *tpcb) -> err_t {
        auto& sem = *static_cast<t*>(user);
        sem.s.up();
        system_os_post(tos::esp82::main_task_prio, 0, 0);
        return ERR_OK;
    }, 1);*/

    tcp_err(data.cl, [](void *user, err_t err) {
        auto& sem = *static_cast<t*>(user);
        //sem.s.up();
        system_os_post(tos::esp82::main_task_prio, 0, 0);
    });

    tcp_accepted(listen_pcb);

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

    if (data.rlen > 0)
    {
        tcp_recved(data.cl, data.rlen);
    }

    tos::println(usart, "read", data.rlen, data.buf);

    err = tcp_write(data.cl, "hello\r\n\r\n", 10, 0);

    if (err != ERR_OK)
    {
        tos::println(usart, "write fail");
        return;
    }

    send:
    tcp_output(data.cl);
    data.s.down();
    if (!data.sent)
    {
        tos::println(usart, "nope");
        goto send;
    }

    tos::println(usart, "wow");

    tcp_recv(data.cl, nullptr);
    err = tcp_close(data.cl);
    tcp_abort(data.cl);

    data.cl = nullptr;

    /*if (err != ERR_OK)
    {
        tos::println(usart, "close failed");
        tcp_abort(data.cl);
    }*/

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