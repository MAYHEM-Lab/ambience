//
// Created by fatih on 7/19/18.
//

#include <tos/print.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <tos/algorithm.hpp>
#include <algorithm>
#include <common/inet/tcp_stream.hpp>
#include <MQTTClient.h>
#include <tos/adapters.hpp>
#include <stdio.h>
#include "sntp.h"

#include "fake_accel.hpp"

#include <umsgpack.hpp>

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

    auto w = open(tos::devs::wifi<0>);
    conn:
    auto res = w.connect("AndroidAP", "12345678");

    tos::println(usart, "connected?", bool(res));
    tos::println(usart, "argument:", arg);

    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn) ICACHE_FLASH_ATTR{
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        lwip_init();
        //axl_init(3);

        sntp_set_timezone(0);
        sntp_setservername(0, "0.tr.pool.ntp.org");
        sntp_init();
        fake_accel accel{ { -1, -1, -1}, { 1, 1, 1 } };

        for (int i = 0; i < 25'000; ++i)
        {
            with(tos::esp82::connect(conn, { { 45, 55, 149, 110 } }, { 1883 }), [&](tos::esp82::tcp_endpoint& conn){
                tos::tcp_stream<tos::esp82::tcp_endpoint> stream {std::move(conn)};
                net_facade net{stream};

                MQTT::Client<net_facade, timer_facade> client{ net };
                MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                data.MQTTVersion = 3;
                data.clientID.cstring = (char*)"tos-client";
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
                static char pbuf[512];
                auto s = accel.sample();

                struct umsgpack_packer_buf *buf = (struct umsgpack_packer_buf*)&pbuf;

                umsgpack_packer_init(buf, sizeof(pbuf));

                umsgpack_pack_map(buf, 3);

                umsgpack_pack_str(buf, (char *) "x", 1);
                umsgpack_pack_float(buf, s.x);

                umsgpack_pack_str(buf, (char *) "y", 1);
                umsgpack_pack_float(buf, s.y);

                umsgpack_pack_str(buf, (char *) "z", 1);
                umsgpack_pack_float(buf, s.z);

                auto str = tos::span<const char>((char*)buf->data, umsgpack_get_length(buf));
                tos::println(usart, str);

                message.payload = (void*)str.data();
                message.payloadlen = str.size();
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