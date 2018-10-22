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
#include "sntp.h"

#include "fake_accel.hpp"
#include <tos/posix.hpp>

#include <cwpack.hpp>
#include <unistd.h>
#include <drivers/common/adxl345.hpp>

struct pair
{
    vec3 v;
    uint32_t base_time;
    uint32_t count;
};

static tos::fixed_fifo<pair, 127> vecs;

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

    static char pbuf[1024];
    tos::msgpack::packer p { pbuf };

    auto put_vec = [](auto& to, pair s)
    {
        auto map = to.insert_arr(5);
        map.insert(s.v.x);
        map.insert(s.v.y);
        map.insert(s.v.z);
        map.insert(s.base_time);
        map.insert(s.count);
    };

    tos::println(*out, vecs.size());
    int len = std::min<int>(20, vecs.size());
    auto arr = p.insert_arr(len);
    for (int i = 0; i < len; ++i)
    {
        put_vec(arr, vecs.pop());
        tos::this_thread::yield();
    }

    message.payload = (void*)p.get().data();
    message.payloadlen = p.get().size();

    auto res = client.publish("tos-sample", message);

    return res;
}

template<class T>
int handle_samples_tcp(T& client)
{
    while (vecs.size() < 20)
    {
        tos::this_thread::yield();
    }

    static char pbuf[768];
    tos::msgpack::packer p { pbuf };

    auto put_vec = [](auto& to, pair s)
    {
        auto map = to.insert_arr(5);
        map.insert(s.v.x);
        map.insert(s.v.y);
        map.insert(s.v.z);
        map.insert(s.base_time);
        map.insert(s.count);
    };

    tos::println(*out, vecs.size());
    int len = std::min<int>(20, vecs.size());
    auto arr = p.insert_arr(len);
    for (int i = 0; i < len; ++i)
    {
        put_vec(arr, vecs.pop());
    }

    client.write(p.get());

    return 0;
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
        vecs.push({ acc.sample(), 0, 0 });
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

    uint32_t time = sntp_get_current_timestamp();
    while (time == 0)
    {
        tos::this_thread::yield();
        time = sntp_get_current_timestamp();
    }

    uint32_t count = 0;
    while (true)
    {
        int x, y, z;
        sens.readAccel(&x, &y, &z);
        vec3 v;
        v.x = x;
        v.y = y;
        v.z = z;
        vecs.push({ v, time, count++ });
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

        tos::launch(fake_task);

        int i = 0;
        while (true)
        {
            auto res = handle_samples_tcp(usart);
            tos::println(usart, "res", res, i++, int(system_get_free_heap_size()));
            tos::this_thread::yield();
        }

        sntp_set_timezone(0);
        sntp_setservername(0, "0.tr.pool.ntp.org");
        sntp_init();

        for (int j = 0; j < 25; ++j)
        {
            bool isconn = false;
            while (!isconn)
                with(tos::esp82::connect(conn, { { 165, 227, 135, 235 } }, { 9993 }), [&](tos::esp82::tcp_endpoint& conn){
                    tos::tcp_stream<tos::esp82::tcp_endpoint> stream {std::move(conn)};
                    //net_facade net{stream};
                    isconn = true;

                    /*auto client = new MQTT::Client<net_facade, timer_facade, 512>(net);
                    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
                    data.MQTTVersion = 3;
                    data.clientID.cstring = (char*)"tos-client";
                    auto rc = client->connect(data);

                    if (rc != 0)
                    {
                        tos::println(usart, "rc from MQTT connect is", rc);
                        return;
                    }

                    tos::println(usart, "MQTT connected");*/
                    if (j == 0)
                    {
                        tos::launch(sample_task);
                    }

                    for (int i = 0; i < 1'000; ++i) {
                        if (stream.disconnected())
                        {
                            tos::println(usart, "disc");
                            return;
                        }

                        auto res = handle_samples_tcp(stream);
                        tos::println(usart, "res", res, j * 1000 + i, int(system_get_free_heap_size()));
                        tos::this_thread::yield();
                    }

                    //delete client;
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