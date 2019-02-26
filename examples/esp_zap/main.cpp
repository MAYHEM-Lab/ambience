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

//#include <flatbuffers/flatbuffers.h>
//#include "req_generated.h"

auto zap_task = []{
    tos::esp82::wifi w;

    conn_:
    auto res = w.connect("cs190b", "cs190bcs190b");
    if (!res) goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();

    auto addr = force_get(wconn.get_addr());
    lwip_init();

    tos::esp82::async_udp_socket udp;
    udp.bind(tos::port_num_t{9993});

    auto handler = [&](tos::lwip::events::recvfrom_t, auto* sock, auto from, tos::lwip::buffer&& buf){
    };

    udp.attach(handler);

    //flatbuffers::FlatBufferBuilder builder(256);
    //zap::cloud::RequestBuilder b(builder);
};

void tos_main()
{
    tos::launch(zap_task);
}
