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
#include <tos/posix.hpp>

#include <umsgpack.hpp>
#include <unistd.h>
#include <drivers/common/adxl345.hpp>

static tos::fixed_fifo<vec3, 127> vecs;

tos::posix::file_desc* out;

int handle_samples(MQTT::Client<net_facade, timer_facade, 512>& client)
{
    while (vecs.size() < 20)
    {
        tos::this_thread::yield();
    }
    MQTT::Message message;
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;

    static char pbuf[512];
    tos::msgpack::packer p { pbuf };

    auto put_vec = [](auto& to, vec3 s)
    {
        auto map = to.insert_map(3);
        map.insert("x", s.x);
        map.insert("y", s.y);
        map.insert("z", s.z);
    };

    tos::println(*out, vecs.size());
    int len = std::min<int>(20, vecs.size());
    auto arr = p.insert_arr(len);
    for (int i = 0; i < len; ++i)
    {
        put_vec(arr, vecs.pop());
    }

    message.payload = (void*)p.get().data();
    message.payloadlen = p.get().size();

    auto res = client.publish("tos-sample", message);

    return res;
}

void fake_task(void* arg)
{
    tos_debug_print("fake stack: %p\n", read_sp());
    uart0_tx_buffer_sync((const uint8_t *)"hello", 5);

    tos::esp82::timer tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    fake_accel acc { {0, 0, 0}, { 1, 1, 1 } };
    tos::println(*out, "Hello!");

    int cnt = 0;
    while (true)
    {
        vecs.push(acc.sample());
        using namespace tos::chrono_literals;
        cnt++;
        if (cnt % 100 == 0)
        {
            tos::println(*out, "runnables:", tos::runnables());
            cnt = 0;
        }
        alarm.sleep_for(10_ms);
    }
}

void sample_task(void* arg)
{
    tos::esp82::gpio g;
    tos::esp82::twim twim{ {4}, {5} };

    tos::adxl345 sens{twim};
    sens.powerOn();
    sens.setRangeSetting(2);

    tos::esp82::timer tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    tos::println(*out, "Hello!");

    while (true)
    {
        int x, y, z;
        sens.readAccel(&x, &y, &z);
        vec3 v;
        v.x = x;
        v.y = y;
        v.z = z;
        vecs.push(v);
        using namespace tos::chrono_literals;
        alarm.sleep_for(10_ms);
    }
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

    tos::posix::wrapper_desc<decltype(usart)> wrapper(usart);
    out = &wrapper;

    tos::print(usart, "\n\n\n\n\n\n");
    tos::println(usart, tos::platform::board_name);
    tos::println(usart, tos::vcs::commit_hash);

    auto w = open(tos::devs::wifi<0>);
    conn:

    auto res = w.connect("Nakedsense.2", "serdar1988");
    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    with (std::move(res), [&](tos::esp82::wifi_connection& conn) ICACHE_FLASH_ATTR{
        conn.wait_for_dhcp();

        with(conn.get_addr(), [&](auto& addr){
            tos::println(usart, "ip:", addr.addr[0], addr.addr[1], addr.addr[2], addr.addr[3]);
        }, tos::ignore);

        lwip_init();

        for (int j = 0; j < 25; ++j)
        {
            bool isconn = false;
            while (!isconn)
                with(tos::esp82::connect(conn, { { 45, 55, 149, 110 } }, { 1883 }), [&](tos::esp82::tcp_endpoint& conn){
                    tos::tcp_stream<tos::esp82::tcp_endpoint> stream {std::move(conn)};
                    net_facade net{stream};
                    isconn = true;

                    auto client = new MQTT::Client<net_facade, timer_facade, 512>(net);
                    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                    data.MQTTVersion = 3;
                    data.clientID.cstring = (char*)"tos-client";
                    auto rc = client->connect(data);

                    if (rc != 0)
                    {
                        tos::println(usart, "rc from MQTT connect is", rc);
                        return;
                    }

                    tos::println(usart, "MQTT connected");
                    if (j == 0)
                    {
                        tos::launch(fake_task);
                    }

                    for (int i = 0; i < 1'000; ++i) {
                        if (stream.disconnected())
                        {
                            tos::println(usart, "disc");
                            return;
                        }

                        auto res = handle_samples(*client);
                        tos::println(usart, "res", res, j * 1000 + i, int(system_get_free_heap_size()));
                        tos::this_thread::yield();
                    }

                    delete client;
                }, [&](auto&){
                    tos::println(usart, "couldn't connect");
                });
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