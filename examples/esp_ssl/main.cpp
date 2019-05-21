//
// Created by fatih on 7/19/18.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/timer.hpp>
#include <arch/usart.hpp>
#include <arch/wifi.hpp>
#include <arch/tcp.hpp>
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
    0x30, 0x3F, 0x31, 0x24, 0x30, 0x22, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x13,
    0x1B, 0x44, 0x69, 0x67, 0x69, 0x74, 0x61, 0x6C, 0x20, 0x53, 0x69, 0x67,
    0x6E, 0x61, 0x74, 0x75, 0x72, 0x65, 0x20, 0x54, 0x72, 0x75, 0x73, 0x74,
    0x20, 0x43, 0x6F, 0x2E, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04,
    0x03, 0x13, 0x0E, 0x44, 0x53, 0x54, 0x20, 0x52, 0x6F, 0x6F, 0x74, 0x20,
    0x43, 0x41, 0x20, 0x58, 0x33
};

static const unsigned char TA0_RSA_N[] = {
    0xDF, 0xAF, 0xE9, 0x97, 0x50, 0x08, 0x83, 0x57, 0xB4, 0xCC, 0x62, 0x65,
    0xF6, 0x90, 0x82, 0xEC, 0xC7, 0xD3, 0x2C, 0x6B, 0x30, 0xCA, 0x5B, 0xEC,
    0xD9, 0xC3, 0x7D, 0xC7, 0x40, 0xC1, 0x18, 0x14, 0x8B, 0xE0, 0xE8, 0x33,
    0x76, 0x49, 0x2A, 0xE3, 0x3F, 0x21, 0x49, 0x93, 0xAC, 0x4E, 0x0E, 0xAF,
    0x3E, 0x48, 0xCB, 0x65, 0xEE, 0xFC, 0xD3, 0x21, 0x0F, 0x65, 0xD2, 0x2A,
    0xD9, 0x32, 0x8F, 0x8C, 0xE5, 0xF7, 0x77, 0xB0, 0x12, 0x7B, 0xB5, 0x95,
    0xC0, 0x89, 0xA3, 0xA9, 0xBA, 0xED, 0x73, 0x2E, 0x7A, 0x0C, 0x06, 0x32,
    0x83, 0xA2, 0x7E, 0x8A, 0x14, 0x30, 0xCD, 0x11, 0xA0, 0xE1, 0x2A, 0x38,
    0xB9, 0x79, 0x0A, 0x31, 0xFD, 0x50, 0xBD, 0x80, 0x65, 0xDF, 0xB7, 0x51,
    0x63, 0x83, 0xC8, 0xE2, 0x88, 0x61, 0xEA, 0x4B, 0x61, 0x81, 0xEC, 0x52,
    0x6B, 0xB9, 0xA2, 0xE2, 0x4B, 0x1A, 0x28, 0x9F, 0x48, 0xA3, 0x9E, 0x0C,
    0xDA, 0x09, 0x8E, 0x3E, 0x17, 0x2E, 0x1E, 0xDD, 0x20, 0xDF, 0x5B, 0xC6,
    0x2A, 0x8A, 0xAB, 0x2E, 0xBD, 0x70, 0xAD, 0xC5, 0x0B, 0x1A, 0x25, 0x90,
    0x74, 0x72, 0xC5, 0x7B, 0x6A, 0xAB, 0x34, 0xD6, 0x30, 0x89, 0xFF, 0xE5,
    0x68, 0x13, 0x7B, 0x54, 0x0B, 0xC8, 0xD6, 0xAE, 0xEC, 0x5A, 0x9C, 0x92,
    0x1E, 0x3D, 0x64, 0xB3, 0x8C, 0xC6, 0xDF, 0xBF, 0xC9, 0x41, 0x70, 0xEC,
    0x16, 0x72, 0xD5, 0x26, 0xEC, 0x38, 0x55, 0x39, 0x43, 0xD0, 0xFC, 0xFD,
    0x18, 0x5C, 0x40, 0xF1, 0x97, 0xEB, 0xD5, 0x9A, 0x9B, 0x8D, 0x1D, 0xBA,
    0xDA, 0x25, 0xB9, 0xC6, 0xD8, 0xDF, 0xC1, 0x15, 0x02, 0x3A, 0xAB, 0xDA,
    0x6E, 0xF1, 0x3E, 0x2E, 0xF5, 0x5C, 0x08, 0x9C, 0x3C, 0xD6, 0x83, 0x69,
    0xE4, 0x10, 0x9B, 0x19, 0x2A, 0xB6, 0x29, 0x57, 0xE3, 0xE5, 0x3D, 0x9B,
    0x9F, 0xF0, 0x02, 0x5D
};

static const unsigned char TA0_RSA_E[] = {
    0x01, 0x00, 0x01
};

static const br_x509_trust_anchor TAs[1] = {
    {
        { (unsigned char *)TA0_DN, sizeof TA0_DN },
        BR_X509_TA_CA,
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

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("mayhem", "z00mz00m");

    tos::println(usart, "connectedd?", bool(res));
    if (!res) {
        goto conn;
    }

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