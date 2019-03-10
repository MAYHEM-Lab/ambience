//
// Created by fatih on 10/19/18.
//


#include <arch/lx106/drivers.hpp>
#include <common/inet/tcp_stream.hpp>

#include <tos/ft.hpp>
#include <tos/devices.hpp>
#include <tos/print.hpp>
#include <tos/version.hpp>
#include <tos/streams.hpp>


#include "common.hpp"
#include "apps.hpp"

static void esp_main()
{
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    tos::esp82::wifi w;
    conn:
    auto res = w.connect("UCSB Wireless Web");
    tos::println(usart, "connected?", bool(res));
    if (!res) goto conn;

    auto& wconn = force_get(res);

    lwip_init();

    tos::esp82::tcp_socket src_sock{wconn, port_num_t{ 9317 }};
    tos::esp82::tcp_socket sink_sock{wconn, port_num_t{ 9318 }};

    if (!src_sock.is_valid() | !sink_sock.is_valid()) {
        tos::println(usart, "can't open tcp socket!");
    }

    auto acceptor = [](auto&, tos::esp82::tcp_endpoint&& ep){
        tos::launch(tos::def_stack,
                    [p = std::make_unique<tos::tcp_stream<tos::esp82::tcp_endpoint>>(std::move(ep))]{
                        source_task(p);
                    });
        return true;
    };

    auto acceptor_sink = [](auto&, tos::esp82::tcp_endpoint&& ep){
        tos::launch(tos::def_stack,
                    [p = std::make_unique<tos::tcp_stream<tos::esp82::tcp_endpoint>>(std::move(ep))]{
                        sink_task(p);
                    });
        return true;
    };

    src_sock.accept(acceptor);
    sink_sock.accept(acceptor_sink);

    tos::this_thread::block_forever();
}

void tos_main()
{
    tos::launch(tos::stack_size_t{2048}, esp_main);
}
