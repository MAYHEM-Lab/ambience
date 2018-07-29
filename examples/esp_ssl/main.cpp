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

#include <stdlib.h>

extern "C"
{
#include <mem.h>
}

extern "C"
{
    void* malloc(size_t sz)
    {
        return os_malloc(sz);
    }

    void free(void* ptr)
    {
        os_free(ptr);
    }

    void* calloc(size_t nitems, size_t size)
    {
        return os_zalloc(nitems * size);
    }

    void* realloc(void* base, size_t sz)
    {
        return os_realloc(base, sz);
    }

    void ax_wdt_feed()
    {
        system_soft_wdt_feed();
        //tos::this_thread::yield();
    }

    void _exit()
    {
        tos::this_thread::exit();
    }
_PTR
_malloc_r (struct _reent *r, size_t sz)
{
    return malloc (sz);
}

void _getpid_r() {}
void _kill_r() {}
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

    lwip_init();

    axl_init(3);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("AndroidAP", "12345678");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn){
        while (!w.wait_for_dhcp());

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        with(tos::esp82::connect_ssl(conn, { { 45, 55, 149, 110 } }, { 443 }), [&](tos::esp82::secure_tcp_endpoint& conn){
            tos::println(usart, "perfect");

            struct{
                void operator()(tos::lwip::events::sent_t, tos::esp82::secure_tcp_endpoint&){
                    sent_sem.up();
                }
                void operator()(tos::lwip::events::recv_t, tos::esp82::secure_tcp_endpoint&, tos::span<const char> buf){
                    if (got)return;
                    got = true;
                    memcpy(::buf, buf.data(), buf.size());
                    ::buf[buf.size()] = 0;
                    read_sem.up();
                }
                void operator()(tos::lwip::events::discon_t, tos::esp82::secure_tcp_endpoint&){

                }

                tos::semaphore sent_sem{0};
                tos::semaphore read_sem{0};
                bool got = false;
            } handler;
            conn.attach(handler);
            conn.send("GET / HTTP/1.1\r\n"
                      "Host: bakirbros.com\r\n"
                      "\r\n");
            handler.sent_sem.down();
            handler.read_sem.down();
            /*tos::tcp_stream stream {std::move(conn)};
            tos::println(stream, "GET /");
            tos::println(stream);

            with (stream.read(buf), [&](auto& res){
                tos::println(usart, res);
            }, [&](auto){
                tos::println(usart, "didn't receive");
            });*/

        }, [&](auto& err){
            tos::println(usart, "couldn't connect", int(err));
        });

        tos::println(usart, "done", int(system_get_free_heap_size()));
    }, [&](auto& err){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    tos::println(usart, (const char*)buf);
    while (true){
        tos::this_thread::yield();
    }
}

void tos_main()
{
    tos::launch(task);
}