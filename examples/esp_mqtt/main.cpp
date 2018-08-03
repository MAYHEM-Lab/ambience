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
#include <MQTTClient.h>

extern "C"
{
#include <mem.h>
}

struct net_facade
{
    tos::tcp_stream<tos::esp82::secure_tcp_endpoint>& str;

    int ALWAYS_INLINE read(unsigned char* buffer, int len, int)
    {
        return with(str.read(tos::span<char>{ (char*)buffer, len }), [](auto& rd) {
            return rd.size();
        }, [](auto& err){
            return 0;
        });
    }

    int ALWAYS_INLINE write(unsigned char* buffer, int len, int)
    {
        str.write({ (char*)buffer, len });
        return len;
    }
};

struct timer_facade
{
public:
    timer_facade()
    {
        interval_end_ms = 0L;
    }

    timer_facade(int ms)
    {
        countdown_ms(ms);
    }

    bool ALWAYS_INLINE expired()
    {
        tos::this_thread::yield();
        return (interval_end_ms > 0L) && (millis() >= interval_end_ms);
    }

    void ALWAYS_INLINE countdown_ms(unsigned long ms)
    {
        interval_end_ms = millis() + ms;
    }

    void ALWAYS_INLINE countdown(int seconds)
    {
        countdown_ms((unsigned long)seconds * 1000L);
    }

    int ALWAYS_INLINE left_ms()
    {
        return interval_end_ms - millis();
    }

private:

    unsigned long ALWAYS_INLINE millis()
    {
        return system_get_time() / 1000;
    }

    unsigned long interval_end_ms;
};

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
void ICACHE_FLASH_ATTR task(void* arg_pt)
{
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);
    usart.enable();

    auto arg = *static_cast<int*>(arg_pt);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("FG", "23111994a");

    tos::println(usart, "connected?", bool(res));
    tos::println(usart, "argument:", arg);

    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn) ICACHE_FLASH_ATTR{
        while (!conn.wait_for_dhcp());

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        lwip_init();
        axl_init(3);

        for (int i = 0; i < 25; ++i)
        {
            with(tos::esp82::connect_ssl(conn, { { 198, 41, 30, 241 } }, { 8883 }), [&](tos::esp82::secure_tcp_endpoint& conn){
                tos::tcp_stream<tos::esp82::secure_tcp_endpoint> stream {std::move(conn)};
                net_facade net{stream};

                MQTT::Client<net_facade, timer_facade> client{ net };
                MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                data.MQTTVersion = 3;
                data.clientID.cstring = (char*)"tos-sample";
                auto rc = client.connect(data);

                if (rc != 0)
                {
                    tos::println(usart, "rc from MQTT connect is", rc);
                    return;
                }
                tos::println(usart, "MQTT connected");

                MQTT::Message message;
                message.qos = MQTT::QOS1;
                message.retained = false;
                message.dup = false;
                message.payload = (void*)"Secure hello From tos@esp8266";
                message.payloadlen = strlen("Secure hello From tos@esp8266") + 1;
                rc = client.publish("tos-sample", message);
                tos::println(usart, "rc from MQTT publish is", rc);

            }, [&](auto& err){
                tos::println(usart, "couldn't connect");
            });
        }

        tos::println(usart, "done", int(system_get_free_heap_size()));
    }, [&](auto& err){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    while (true){
        tos::this_thread::yield();
    }
}

int x;

void tos_main()
{
    x = 42;
    tos::launch(task, &x);
}