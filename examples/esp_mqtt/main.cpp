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
    tos::tcp_stream& str;

    int ICACHE_FLASH_ATTR read(unsigned char* buffer, int len, int)
    {
        return with(str.read(tos::span<char>{ (char*)buffer, len }), [](auto& rd) {
            return rd.size();
        }, [](auto& err){
            return 0;   
        });
    }

    int ICACHE_FLASH_ATTR write(unsigned char* buffer, int len, int)
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

    bool ICACHE_FLASH_ATTR expired()
    {
        return (interval_end_ms > 0L) && (millis() >= interval_end_ms);
    }

    void ICACHE_FLASH_ATTR countdown_ms(unsigned long ms)
    {
        interval_end_ms = millis() + ms;
    }

    void ICACHE_FLASH_ATTR countdown(int seconds)
    {
        countdown_ms((unsigned long)seconds * 1000L);
    }

    int ICACHE_FLASH_ATTR left_ms()
    {
        return interval_end_ms - millis();
    }

private:

    unsigned long ICACHE_FLASH_ATTR millis()
    {
        return system_get_time() / 1000;
    }

    unsigned long interval_end_ms;
};

char buf[512];
void ICACHE_FLASH_ATTR task()
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

    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn){
        while (!w.wait_for_dhcp());

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        lwip_init();

        with(tos::esp82::connect(conn, { { 198, 41, 30, 241 } }, { 1883 }), [&](tos::esp82::tcp_endpoint& conn){
            tos::tcp_stream stream {std::move(conn)};
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
            message.payload = (void*)"Hello From tos@esp8266";
            message.payloadlen = strlen("Hello From tos@esp8266") + 1;
            rc = client.publish("tos-sample", message);
            tos::println(usart, "rc from MQTT publish is", rc);

        }, [&](auto& err){
            tos::println(usart, "couldn't connect");
        });

        tos::println(usart, "done", int(system_get_free_heap_size()));
    }, [&](auto& err){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });

    while (true){
        tos::this_thread::yield();
    }
}

void tos_main()
{
    tos::launch(task);
}