//
// Created by fatih on 7/19/18.
//

#include <tos/print.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <tos/algorithm.hpp>
#include <common/inet/tcp_stream.hpp>
#include <MQTTClient.h>
#include <tos/adapters.hpp>
#include <stdio.h>
#include "resources.hpp"

#include <tos/posix.hpp>
#include <lwip_sntp/sntp.h>

int handle_samples(MQTT::Client<net_facade, timer_facade, 512>& client)
{
    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;

    static char pbuf[1024];
    tos::span<const char>* p;

    message.payload = (void*)p->data();
    message.payloadlen = p->size();

    auto res = client.publish("tos-sample", message);

    return res;
}

static sntp_res_t tm;
uint64_t get_time()
{
    auto now = system_get_time();
    auto diff_ms = (now - tm.system_time) / 1000;
    return tm.sntp_time + diff_ms;
}

template <class T>
void send(T& usart, tos::esp82::wifi_connection& conn, uint64_t time)
{
    bool isconn = false;
    while (!isconn)
        with(tos::esp82::connect_ssl(conn, { { 128, 111, 45, 13 } }, { 8883 }, private_key, me_cert, certificate), [&](tos::esp82::secure_tcp_endpoint& conn){
            tos::tcp_stream<tos::esp82::secure_tcp_endpoint> stream {std::move(conn)};
            net_facade net{stream};
            isconn = true;

            auto client = new MQTT::Client<net_facade, timer_facade, 512>(net);
            MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
            data.MQTTVersion = 3;
            data.clientID.cstring = (char*)"tos_mcu";
            auto rc = client->connect(data);

            if (rc != 0)
            {
                tos::println(usart, "rc from MQTT connect is", rc);
                return;
            }

            tos::println(usart, "MQTT connected");

            MQTT::Message message;
            message.qos = MQTT::QOS0;
            message.retained = false;
            message.dup = false;

            static char mbuf[512];
            auto sec = int (time / 1000);
            auto msec = int (time % 1000);
            sprintf(mbuf, R"__({ "data" : { "message" : [ %d, %d ] } })__", sec, msec);
            message.payload = (void*)mbuf;
            message.payloadlen = strlen(mbuf);

            auto res = client->publish("hello/device", message);

            tos::println(usart, "pub:", res);

            delete client;
        }, [&](auto&){
            tos::println(usart, "couldn't connect");
        });
}

void ICACHE_FLASH_ATTR task(void* arg_pt)
{
    tos_debug_print("task stack: %p\n", read_sp());
    using namespace tos::tos_literals;

    constexpr auto usconf = tos::usart_config()
            .add(115200_baud_rate)
            .add(tos::usart_parity::disabled)
            .add(tos::usart_stop_bit::one);

    auto usart = open(tos::devs::usart<0>, usconf);

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    auto w = open(tos::devs::wifi<0>);
    conn:

    auto res = w.connect("NETGEAR95", "youngraven422");
    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn) ICACHE_FLASH_ATTR{
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        axl_init(1);
        lwip_init();

        sntp_set_timezone(0);
        tos::ipv4_addr_t taddr { 128, 111, 1, 5 };
        ip_addr_t addr;
        memcpy(&addr.addr, &taddr, 4);
        sntp_setserver(0, &addr);
        sntp_init();

        tm = do_sntp_request();

        tos::println(usart, int(tm.sntp_time / 1000), int(sntp_get_current_timestamp()), system_get_time());

        for (int j = 0; j < 100; ++j)
        {
            send(usart, conn, get_time());
        }

        tos::println(usart, "done");
    }, [&](auto& err){
        tos::println(usart, "uuuh, shouldn't have happened!");
    });
}

void tos_main()
{
    tos_debug_print("main stack: %p\n", read_sp());
    tos::launch(task);
}