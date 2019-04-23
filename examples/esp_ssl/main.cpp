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
#include <tos_arch.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>

#include <bearssl.h>
#include <lwip_sntp/sntp.h>

br_ssl_client_context sc;
br_x509_minimal_context xc;
unsigned char iobuf[BR_SSL_BUFSIZE_MONO];
br_sslio_context ioc;

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
        tos_debug_print("heap: %d\n", int(int(system_get_free_heap_size())));

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
    tos::this_thread::yield();
    auto ptr = static_cast<tcp_ptr>(ctx);
    tos_debug_print("write %p %d\n", buf, int(len));
    if (ptr->disconnected())
        return -1;
    return ptr->write({ (const char*)buf, len });
}

extern "C" void optimistic_yield(uint32_t){
    static uint32_t last_yield = 0;
    if (system_get_time() - last_yield > 1'000'000  || last_yield == 0)
    {
        tos_debug_print("yielding\n");
        system_soft_wdt_feed();
        tos::this_thread::yield();
        last_yield = system_get_time();
    }
}

static const unsigned char TA0_DN[] = {
    0x30, 0x13, 0x31, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x04, 0x03, 0x13,
    0x08, 0x62, 0x61, 0x6B, 0x69, 0x72, 0x2E, 0x69, 0x6F
};

static const unsigned char TA0_RSA_N[] = {
    0xF4, 0xA4, 0xDF, 0x8A, 0x09, 0xA9, 0xDE, 0x43, 0xE1, 0x16, 0x9C, 0x47,
    0xAD, 0xC2, 0x95, 0xA3, 0x54, 0x89, 0x82, 0xCA, 0xFB, 0xC4, 0x61, 0xC0,
    0xF8, 0xFD, 0x2F, 0xB6, 0x29, 0x71, 0x71, 0xDB, 0xB8, 0x78, 0x13, 0xCA,
    0xC2, 0x9E, 0x80, 0xF8, 0x9F, 0xDB, 0x42, 0x16, 0xFE, 0x98, 0x11, 0x2A,
    0xEC, 0x00, 0x78, 0xEE, 0x06, 0x9B, 0x60, 0x17, 0x76, 0x9D, 0xD7, 0x3B,
    0x90, 0x77, 0xC9, 0x62, 0xA3, 0xB3, 0xE7, 0xB7, 0x87, 0x7B, 0x36, 0x5C,
    0x31, 0x6D, 0xA4, 0xB2, 0x4D, 0xF4, 0x68, 0xC3, 0x75, 0xB0, 0x84, 0xF7,
    0xF8, 0x97, 0x1B, 0x72, 0xC3, 0xAF, 0xB9, 0xFE, 0xE2, 0x15, 0x08, 0xEC,
    0x1C, 0x94, 0x5B, 0xC2, 0x16, 0xD8, 0x66, 0x5E, 0x17, 0x42, 0x8E, 0xED,
    0xFE, 0x59, 0x63, 0x0F, 0xD0, 0x48, 0xC3, 0x9A, 0xD2, 0x88, 0xEB, 0x98,
    0x08, 0xD6, 0xE0, 0x4A, 0x11, 0xAA, 0xCF, 0x65, 0x06, 0xCC, 0xF2, 0xC4,
    0x79, 0xCE, 0xF5, 0x06, 0x95, 0x57, 0x5A, 0x8A, 0xEC, 0x7C, 0x48, 0xAC,
    0x12, 0x7C, 0x6F, 0x92, 0x12, 0xF5, 0x80, 0xD0, 0xE0, 0x7F, 0xAE, 0xBD,
    0xAC, 0x23, 0x61, 0x1B, 0xD4, 0xD0, 0xE4, 0xE6, 0x59, 0xC9, 0xB8, 0x54,
    0x3D, 0xCE, 0xB4, 0x53, 0x4B, 0xE9, 0xF0, 0x89, 0xCE, 0x37, 0xCD, 0x53,
    0x9C, 0x6B, 0x3B, 0x1A, 0x4C, 0x81, 0x87, 0x02, 0x28, 0x6F, 0xD4, 0x91,
    0x88, 0x65, 0x57, 0x00, 0xD3, 0x9C, 0x52, 0xF2, 0x96, 0xC0, 0x99, 0xE6,
    0x69, 0xE1, 0x13, 0x6C, 0xF4, 0x9C, 0xA8, 0x7A, 0xD9, 0xC8, 0xFF, 0x68,
    0xA9, 0x6B, 0xD2, 0x2E, 0x9D, 0x8B, 0x66, 0x19, 0x33, 0x54, 0x86, 0xDF,
    0x9A, 0x0A, 0xBD, 0xBB, 0x87, 0xBF, 0xAE, 0x41, 0x0D, 0xFF, 0xAE, 0x0A,
    0x52, 0xAD, 0xBE, 0xC2, 0x17, 0x17, 0x80, 0xAE, 0x7F, 0x4F, 0x29, 0x28,
    0x55, 0xB4, 0x55, 0x65
};

static const unsigned char TA0_RSA_E[] = {
    0x01, 0x00, 0x01
};

static const br_x509_trust_anchor TAs[1] = {
    {
        { (unsigned char *)TA0_DN, sizeof TA0_DN },
        0,
        {
            BR_KEYTYPE_RSA,
            { .rsa = {
                (unsigned char *)TA0_RSA_N, sizeof TA0_RSA_N,
                (unsigned char *)TA0_RSA_E, sizeof TA0_RSA_E,
            } }
        }
    }
};


template <class BaseT>
struct ssl_wrapper : public tos::self_pointing<ssl_wrapper<BaseT>>
{
public:
    ssl_wrapper(tos::tcp_stream<tos::esp82::tcp_endpoint>& base) : m_base(base) {
        br_ssl_client_init_full(&sc, &xc, TAs, 1);
        br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof iobuf, 1);
        tos_debug_print("err %d\n", int(br_ssl_engine_last_error(&sc.eng)));

        if (!br_ssl_client_reset(&sc, "bakir.io", 0))
        {
            tos_debug_print("ssl_reset failed\n");
        }

        br_sslio_init(&ioc, &sc.eng, sock_read, &m_base, sock_write, &m_base);
    }

    size_t write(tos::span<const char> buf)
    {
        auto res = br_sslio_write_all(&ioc, buf.data(), buf.size());
        if (res != 0) {
            tos_debug_print("err %d %d\n", res, int(br_ssl_engine_last_error(&sc.eng)));
            return 0;
        }
        br_sslio_flush(&ioc);
        return buf.size();
    }

    tos::expected<tos::span<char>, int> read(tos::span<char> buf)
    {
        auto len = br_sslio_read(&ioc, buf.data(), buf.size());
        if (len == -1)
        {
            tos_debug_print("errr %d\n", int(br_ssl_engine_last_error(&sc.eng)));
            return tos::unexpected(br_ssl_engine_last_error(&sc.eng));
            // error
        }
        return buf.slice(0, len);
    }

private:

    tos::tcp_stream<tos::esp82::tcp_endpoint>& m_base;
};

char buf[512];
void task()
{
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    lwip_init();

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("cs190b", "cs190bcs190b");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn){
        tos::println(usart, "in");
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        sntp_set_timezone(0);
        tos::ipv4_addr_t taddr { 128, 111, 1, 5 };
        ip_addr_t addr;
        memcpy(&addr.addr, &taddr, 4);
        sntp_setserver(0, &addr);
        sntp_init();

        do_sntp_request();

        for (int i = 0; i < 1500; ++i)
        {
            with(tos::esp82::connect(conn, { { 3, 122, 138, 41 } }, { 443 }), [&](tos::esp82::tcp_endpoint& conn){
                tos::println(usart, "perfect");

                tos::tcp_stream<tos::esp82::tcp_endpoint> stream(std::move(conn));

                ssl_wrapper<decltype((stream))> ssl(stream);
                system_soft_wdt_stop();

                ssl.write("GET / HTTP/1.1\r\n"
                             "Host: bakir.io\r\n"
                             "Connection: close\r\n"
                             "\r\n");

                while (true)
                {
                    auto res = ssl.read(buf);
                    if (!res) break;
                    auto r = force_get(res);
                    tos::print(usart, r);
                    tos::this_thread::yield();
                }
                tos::println(usart);
                system_soft_wdt_restart();
            }, [&](auto& err){
                tos::println(usart, "couldn't connect", int(err));
            });

            tos::println(usart);
            tos::println(usart, "done", i, int(system_get_free_heap_size()));
        }
    }, [&](auto&){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    tos::this_thread::block_forever();
}

tos::stack_storage<1024 * 6> stk;
void tos_main()
{
    tos::launch(stk, task);
}