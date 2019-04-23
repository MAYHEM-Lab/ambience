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

br_ssl_client_context sc;
br_x509_minimal_context xc;
unsigned char iobuf[BR_SSL_BUFSIZE_MONO];
br_sslio_context ioc;

template <class BaseT>
struct ssl_wrapper : public tos::self_pointing<ssl_wrapper<BaseT>>
{
public:
    template <class BaseU>
    ssl_wrapper(BaseU&& base) : m_base(std::forward<BaseU>(base)) {
        br_ssl_client_init_full(&sc, &xc, nullptr, 0);
        br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof iobuf, 1);

        br_ssl_client_reset(&sc, "bakir.io", 0);

        br_sslio_init(&ioc, &sc.eng, sock_read, &m_base, sock_write, &m_base);
    }

    size_t write(tos::span<const char> buf)
    {
        auto res = br_sslio_write_all(&ioc, buf.data(), buf.size());
        if (res == -1) return 0;
        br_sslio_flush(&ioc);
        return buf.size();
    }

    tos::span<char> read(tos::span<char> buf)
    {
        auto len = br_sslio_read(&ioc, buf.data(), buf.size());
        return buf.slice(0, len);
    }

private:

    BaseT m_base;
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
    auto res = w.connect("WIFI", "PASS");

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn){
        tos::println(usart, "in");
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        for (int i = 0; i < 1500; ++i)
        {
            with(tos::esp82::connect(conn, { { 3, 122, 138, 41 } }, { 443 }), [&](tos::esp82::tcp_endpoint& conn){
                tos::println(usart, "perfect");

                tos::tcp_stream<tos::esp82::tcp_endpoint> stream(std::move(conn));

                ssl_wrapper<decltype((stream))> ssl(stream);

                ssl.write("GET / HTTP/1.1\r\n"
                             "Host: bakirbros.com\r\n"
                             "Connection: close\r\n"
                             "\r\n");

                tos::println(usart);

                while (true)
                {
                    auto res = ssl.read(buf);
                    tos::print(usart, res);
                    tos::this_thread::yield();
                }
                tos::println(usart);
            }, [&](auto& err){
                tos::println(usart, "couldn't connect", int(err));
            });

            tos::println(usart);
            tos::println(usart, "done", i, int(system_get_free_heap_size()));
        }
    }, [&](auto&){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    while (true){
        tos::this_thread::yield();
    }
}

void tos_main()
{
    tos::launch(tos::alloc_stack, task);
}