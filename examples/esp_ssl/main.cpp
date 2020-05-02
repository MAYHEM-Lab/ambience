//
// Created by fatih on 7/19/18.
//

#include <arch/tcp.hpp>
#include <arch/timer.hpp>
#include <arch/usart.hpp>
#include <arch/wifi.hpp>
#include <bearssl.h>
#include <common/inet/tcp_stream.hpp>
#include <lwip/init.h>
#include <lwip_sntp/sntp.h>
#include <tos/devices.hpp>
#include <tos/fixed_fifo.hpp>
#include <tos/ft.hpp>
#include <tos/mutex.hpp>
#include <tos/print.hpp>
#include <tos/semaphore.hpp>
#include <tos/utility.hpp>
#include <tos_arch.hpp>

br_ssl_client_context sc;
br_x509_minimal_context xc;
unsigned char iobuf[BR_SSL_BUFSIZE_MONO];
br_sslio_context ioc;

using tcp_ptr = tos::tcp_stream<tos::esp82::tcp_endpoint>*;
/*
 * Low-level data read callback for the simplified SSL I/O API.
 */
static int sock_read(void* ctx, unsigned char* buf, size_t len) {
    auto ptr = static_cast<tcp_ptr>(ctx);
    for (;;) {
        auto res = ptr->read({(uint8_t*)buf, len});
        tos_debug_print("heap: %d\n", int(int(system_get_free_heap_size())));

        if (!res) {
            return -1;
        }

        auto r = force_get(res);

        if (r.size() == 0) {
            continue;
        }

        return r.size();
    }
}

/*
 * Low-level data write callback for the simplified SSL I/O API.
 */
static int sock_write(void* ctx, const unsigned char* buf, size_t len) {
    tos::this_thread::yield();
    auto ptr = static_cast<tcp_ptr>(ctx);
    tos_debug_print("write %p %d\n", buf, int(len));
    if (ptr->disconnected())
        return -1;
    return ptr->write({(const uint8_t*)buf, len});
}

extern "C" void optimistic_yield(uint32_t) {
    static uint32_t last_yield = 0;
    if (system_get_time() - last_yield > 1'000'000 || last_yield == 0) {
        tos_debug_print("yielding\n");
        system_soft_wdt_feed();
        tos::this_thread::yield();
        last_yield = system_get_time();
    }
}
static const unsigned char TA0_DN[] = {
    0x30, 0x61, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
    0x02, 0x55, 0x53, 0x31, 0x15, 0x30, 0x13, 0x06, 0x03, 0x55, 0x04, 0x0A,
    0x13, 0x0C, 0x44, 0x69, 0x67, 0x69, 0x43, 0x65, 0x72, 0x74, 0x20, 0x49,
    0x6E, 0x63, 0x31, 0x19, 0x30, 0x17, 0x06, 0x03, 0x55, 0x04, 0x0B, 0x13,
    0x10, 0x77, 0x77, 0x77, 0x2E, 0x64, 0x69, 0x67, 0x69, 0x63, 0x65, 0x72,
    0x74, 0x2E, 0x63, 0x6F, 0x6D, 0x31, 0x20, 0x30, 0x1E, 0x06, 0x03, 0x55,
    0x04, 0x03, 0x13, 0x17, 0x44, 0x69, 0x67, 0x69, 0x43, 0x65, 0x72, 0x74,
    0x20, 0x47, 0x6C, 0x6F, 0x62, 0x61, 0x6C, 0x20, 0x52, 0x6F, 0x6F, 0x74,
    0x20, 0x43, 0x41
};

static const unsigned char TA0_RSA_N[] = {
    0xE2, 0x3B, 0xE1, 0x11, 0x72, 0xDE, 0xA8, 0xA4, 0xD3, 0xA3, 0x57, 0xAA,
    0x50, 0xA2, 0x8F, 0x0B, 0x77, 0x90, 0xC9, 0xA2, 0xA5, 0xEE, 0x12, 0xCE,
    0x96, 0x5B, 0x01, 0x09, 0x20, 0xCC, 0x01, 0x93, 0xA7, 0x4E, 0x30, 0xB7,
    0x53, 0xF7, 0x43, 0xC4, 0x69, 0x00, 0x57, 0x9D, 0xE2, 0x8D, 0x22, 0xDD,
    0x87, 0x06, 0x40, 0x00, 0x81, 0x09, 0xCE, 0xCE, 0x1B, 0x83, 0xBF, 0xDF,
    0xCD, 0x3B, 0x71, 0x46, 0xE2, 0xD6, 0x66, 0xC7, 0x05, 0xB3, 0x76, 0x27,
    0x16, 0x8F, 0x7B, 0x9E, 0x1E, 0x95, 0x7D, 0xEE, 0xB7, 0x48, 0xA3, 0x08,
    0xDA, 0xD6, 0xAF, 0x7A, 0x0C, 0x39, 0x06, 0x65, 0x7F, 0x4A, 0x5D, 0x1F,
    0xBC, 0x17, 0xF8, 0xAB, 0xBE, 0xEE, 0x28, 0xD7, 0x74, 0x7F, 0x7A, 0x78,
    0x99, 0x59, 0x85, 0x68, 0x6E, 0x5C, 0x23, 0x32, 0x4B, 0xBF, 0x4E, 0xC0,
    0xE8, 0x5A, 0x6D, 0xE3, 0x70, 0xBF, 0x77, 0x10, 0xBF, 0xFC, 0x01, 0xF6,
    0x85, 0xD9, 0xA8, 0x44, 0x10, 0x58, 0x32, 0xA9, 0x75, 0x18, 0xD5, 0xD1,
    0xA2, 0xBE, 0x47, 0xE2, 0x27, 0x6A, 0xF4, 0x9A, 0x33, 0xF8, 0x49, 0x08,
    0x60, 0x8B, 0xD4, 0x5F, 0xB4, 0x3A, 0x84, 0xBF, 0xA1, 0xAA, 0x4A, 0x4C,
    0x7D, 0x3E, 0xCF, 0x4F, 0x5F, 0x6C, 0x76, 0x5E, 0xA0, 0x4B, 0x37, 0x91,
    0x9E, 0xDC, 0x22, 0xE6, 0x6D, 0xCE, 0x14, 0x1A, 0x8E, 0x6A, 0xCB, 0xFE,
    0xCD, 0xB3, 0x14, 0x64, 0x17, 0xC7, 0x5B, 0x29, 0x9E, 0x32, 0xBF, 0xF2,
    0xEE, 0xFA, 0xD3, 0x0B, 0x42, 0xD4, 0xAB, 0xB7, 0x41, 0x32, 0xDA, 0x0C,
    0xD4, 0xEF, 0xF8, 0x81, 0xD5, 0xBB, 0x8D, 0x58, 0x3F, 0xB5, 0x1B, 0xE8,
    0x49, 0x28, 0xA2, 0x70, 0xDA, 0x31, 0x04, 0xDD, 0xF7, 0xB2, 0x16, 0xF2,
    0x4C, 0x0A, 0x4E, 0x07, 0xA8, 0xED, 0x4A, 0x3D, 0x5E, 0xB5, 0x7F, 0xA3,
    0x90, 0xC3, 0xAF, 0x27
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
template<class BaseT>
struct ssl_wrapper : public tos::self_pointing<ssl_wrapper<BaseT>> {
public:
    ssl_wrapper(tos::tcp_stream<tos::esp82::tcp_endpoint>& base)
        : m_base(base) {
        br_ssl_client_init_full(&sc, &xc, TAs, 1);
        br_ssl_engine_set_buffer(&sc.eng, iobuf, sizeof iobuf, 1);
        tos_debug_print("err %d\n", int(br_ssl_engine_last_error(&sc.eng)));

        if (!br_ssl_client_reset(&sc, "example.com", 0)) {
            tos_debug_print("ssl_reset failed\n");
        }

        br_sslio_init(&ioc, &sc.eng, sock_read, &m_base, sock_write, &m_base);
    }

    size_t write(tos::span<const char> buf) {
        auto res = br_sslio_write_all(&ioc, buf.data(), buf.size());
        if (res != 0) {
            tos_debug_print("err %d %d\n", res, int(br_ssl_engine_last_error(&sc.eng)));
            return 0;
        }
        br_sslio_flush(&ioc);
        return buf.size();
    }

    tos::expected<tos::span<char>, int> read(tos::span<char> buf) {
        auto len = br_sslio_read(&ioc, buf.data(), buf.size());
        if (len == -1) {
            tos_debug_print("errr %d\n", int(br_ssl_engine_last_error(&sc.eng)));
            return tos::unexpected(br_ssl_engine_last_error(&sc.eng));
            // error
        }
        return buf.slice(0, len);
    }

private:
    tos::tcp_stream<tos::esp82::tcp_endpoint>& m_base;
};

extern "C" {
uint32_t tos_rand_source() {
    return 4;
}
}

char buf[512];
void task() {
    system_update_cpu_freq(SYS_CPU_160MHZ);
    using namespace tos::tos_literals;

    auto usart = open(tos::devs::usart<0>, tos::uart::default_115200);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);

    tos::esp82::wifi w;
conn:
    auto res = w.connect("bkr", "@bakir123");

    tos::println(usart, "connectedd?", bool(res));
    if (!res) {
        goto conn;
    }

    auto& wconn = force_get(res);

    tos::println(usart, "in");
    wconn.wait_for_dhcp();

    with(
        wconn.get_addr(),
        [&](auto& addr) {
            tos::println(
                usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        },
        tos::ignore);

    sntp_set_timezone(0);
    tos::ipv4_addr_t taddr{128, 111, 1, 5};
    ip_addr_t addr;
    memcpy(&addr.addr, &taddr, 4);
    sntp_setserver(0, &addr);
    sntp_init();

    do_sntp_request();

    for (int i = 0; i < 1500; ++i) {
        auto conn =
            tos::esp82::connect(wconn, tos::parse_ipv4_address("93.184.216.34"), {443});

        if (!conn) {
            tos::println(usart, "couldn't connect");
            continue;
        }

        tos::println(usart, "perfect");

        tos::tcp_stream<tos::esp82::tcp_endpoint> stream(force_get(std::move(conn)));

        ssl_wrapper<decltype((stream))> ssl(stream);
        system_soft_wdt_stop();

        ssl.write("GET / HTTP/1.1\r\n"
                  "Host: example.com\r\n"
                  "Connection: close\r\n"
                  "\r\n");

        while (true) {
            auto read_res = ssl.read(buf);
            if (!read_res)
                break;
            auto r = force_get(read_res);
            tos::print(usart, r);
            tos::this_thread::yield();
        }

        tos::println(usart);
        system_soft_wdt_restart();

        tos::println(usart);
        tos::println(usart, "done", i, int(system_get_free_heap_size()));
    }


    tos::this_thread::block_forever();
}

tos::stack_storage<1024 * 6> stk;
void tos_main() {
    tos::launch(stk, task);
}