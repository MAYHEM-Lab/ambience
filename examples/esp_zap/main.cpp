//
// Created by fatih on 2/22/19.
//

#include <tos/devices.hpp>
#include <tos/ft.hpp>
#include <tos/semaphore.hpp>
#include <tos/print.hpp>
#include <tos/mutex.hpp>
#include <tos/utility.hpp>

#include <arch/lx106/drivers.hpp>
#include <tos/version.hpp>
#include <tos/fixed_fifo.hpp>

#include <lwip/init.h>
#include <common/inet/tcp_stream.hpp>
#include <arch/lx106/udp.hpp>
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
    lwip_init();

    return std::make_pair(w, std::move(wconn));
}

auto zap_task = []{
    auto [w, wconn] = wifi_connect();

    alignas(std::max_align_t) uint8_t buf[128];
    //alignas(std::max_align_t) uint8_t sbuf[] = {'h', 'e', 'l', 'l', 'o'};

    tos::udp_endpoint_t ep;
    ep.addr = tos::parse_ip("169.231.9.60");
    ep.port = {9993};
    auto timer = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, timer);

    tos_debug_print("\nhi\n");
    int i = 0;
    for (;;)
    {
        tos::esp82::async_udp_socket udp;

        auto r = udp.bind(tos::port_num_t{9993});
        if (!r)
        {
            goto end;
        }

        {
            tos::omemory_stream str(buf);
            tos::println(str, "hello", i++);
            udp.send_to({(const uint8_t*)str.get().data(), str.get().size()}, ep);
            //udp.send_to(sbuf, ep);
        }
        //tos_debug_print("\nafter send\n");

        end:
        using namespace std::chrono_literals;
        alarm.sleep_for(10ms);
    }
};

void tos_main()
{
    tos::launch(zap_task);
}
