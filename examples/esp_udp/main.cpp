//
// Created by fatih on 2/22/19.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>
#include <arch/udp.hpp>
#include <tos/mem_stream.hpp>
#include <tos/future.hpp>

auto wifi_connect()
{
    tos::esp82::wifi w;

    conn_:
    auto res = w.connect("cs190b", "cs190bcs190b");
    if (!res) goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    return std::make_pair(w, std::move(wconn));
}

auto udp_task = []{
    auto [w, wconn] = wifi_connect();

    tos::udp_endpoint_t ep{
        .addr = tos::parse_ip("169.231.9.60"),
        .port = { 9993 }
    };

    auto timer = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, timer);

    int i = 0;
    for (;;)
    {
        tos::esp82::async_udp_socket udp;

        auto r = udp.bind(tos::port_num_t{9993});
        if (r)
        {
            alignas(std::max_align_t) uint8_t buf[128];
            tos::omemory_stream str(buf);
            tos::println(str, "hello", i++);
            udp.send_to(tos::raw_cast<const uint8_t>(str.get()), ep);
        }

        using namespace std::chrono_literals;
        alarm.sleep_for(10ms);
    }
};

void tos_main()
{
    tos::launch(tos::alloc_stack, udp_task);
}
