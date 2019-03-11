//
// Created by fatih on 3/10/19.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "keys.hpp"
#include <bearssl.h>

#include <arch/lx106/drivers.hpp>
#include <arch/lx106/tcp.hpp>
#include <common/usart.hpp>
#include <tos/print.hpp>
#include <common/inet/tcp_stream.hpp>

uint32_t last_yield = 0;

extern "C" void optimistic_yield(uint32_t){
    if (system_get_time() - last_yield > 1'000'000  || last_yield == 0)
    {
        tos_debug_print("yielding\n");
        system_soft_wdt_feed();
        tos::this_thread::yield();
        last_yield = system_get_time();
    }
}

using tcp_ptr = tos::tcp_stream<tos::esp82::tcp_endpoint>*;
/*
 * Low-level data read callback for the simplified SSL I/O API.
 */
static int
sock_read(void *ctx, unsigned char *buf, size_t len)
{
    auto ptr = static_cast<tcp_ptr>(ctx);
    for (;;) {
        auto res = ptr->read({ (char*)buf, len });

        if (!res)
        {
            return -1;
        }

        auto r = force_get(res);

        if (r.size() == 0)
        {
            continue;
        }

        return r.size();
    }
}

/*
 * Low-level data write callback for the simplified SSL I/O API.
 */
static int
sock_write(void *ctx, const unsigned char *buf, size_t len)
{
    auto ptr = static_cast<tcp_ptr>(ctx);
    for (;;) {
        tos_debug_print("write %d\n", int(len));
        if (ptr->disconnected())
            return -1;
        return ptr->write({ (const char*)buf, len });
    }
}

/*
 * Sample HTTP response to send.
 */
static const char *HTTP_RES =
        "HTTP/1.0 200 OK\r\n"
        "Content-Length: 46\r\n"
        "Connection: close\r\n"
        "Content-Type: text/html; charset=iso-8859-1\r\n"
        "\r\n"
        "<html>\r\n"
        "<body>\r\n"
        "<p>Test!</p>\r\n"
        "</body>\r\n"
        "</html>\r\n";

unsigned char iobuf[5000];
br_ssl_server_context sc;
template <class StreamT, class UsartT>
void handle_sock(StreamT& p, UsartT& usart)
{
    tos::println(usart, "got req");
    br_sslio_context ioc;

    br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof iobuf, 1);
    br_ssl_server_reset(&sc);

    br_sslio_init(&ioc, &sc.eng, sock_read, &p, sock_write, &p);

    system_update_cpu_freq(SYS_CPU_160MHZ);
    system_soft_wdt_stop();
    auto lcwn = 0;
    for (;;) {
        unsigned char x;
        if (br_sslio_read(&ioc, &x, 1) < 0) {
            goto client_drop;
        }
        if (x == 0x0D) {
            continue;
        }
        if (x == 0x0A) {
            if (lcwn) {
                break;
            }
            lcwn = 1;
        } else {
            lcwn = 0;
        }
    }
    system_soft_wdt_restart();

    system_update_cpu_freq(SYS_CPU_80MHZ);

    br_sslio_write_all(&ioc, HTTP_RES, strlen(HTTP_RES));
    br_sslio_close(&ioc);

    client_drop:
    auto err = br_ssl_engine_last_error(&sc.eng);
    if (err == 0) {
        tos::println(usart, "SSL closed (correctly).");
    } else {
        tos::println(usart, "SSL error:", err);
    }
}

extern rst_info rst;
tos::stack_storage<1024 * 8> s;
void server()
{
    using namespace tos;
    using namespace tos::tos_literals;
    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("UCSB Wireless Web");
    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    auto& wconn = force_get(res);

    lwip_init();

    br_ssl_server_init_full_rsa(&sc, CHAIN, CHAIN_LEN, &RSA);

    tos::esp82::tcp_socket src_sock{wconn, port_num_t{ 9993 }};

    tos::println(usart, "reset reason:", int(rst.reason), (void*)rst.epc1);
    auto acceptor = [&](auto&, tos::esp82::tcp_endpoint&& ep){
        tos::launch(s,
                    [p = std::make_unique<tos::tcp_stream<tos::esp82::tcp_endpoint>>(std::move(ep)), &usart]{
                        handle_sock(*p, usart);
                    });
        return true;
    };
    src_sock.accept(acceptor);
    tos::this_thread::block_forever();
}

void tos_main()
{
    tos::launch(tos::def_stack, server);
}