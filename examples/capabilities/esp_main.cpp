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

#include "monotonic_clock.hpp"
#include <numeric>

static void time_emsha()
{
    std::array<uint8_t , 32> in;
    std::iota(in.begin(), in.end(), 0);

    std::vector<std::chrono::microseconds> expr;
    expr.reserve(100);
    caps::emsha::signer s{"foo"};
    for (int j = 0; j < 100; ++j)
    {
        auto begin = tos::high_resolution_clock::now();
        for (int i = 0; i < 1000; ++i)
        {
            volatile auto sign = s.sign(in);
        }
        auto end = tos::high_resolution_clock::now();
        tos::this_thread::yield();
        expr.emplace_back(end - begin);
    }

    for (auto& r : expr)
    {
        tos_debug_print("%u\n", size_t(r.count()));
    }
}

struct bench_range_end_t {};

struct bench_range
{
    constexpr bool operator==(bench_range_end_t)
    {
        return m_i == m_end;
    }

    constexpr bool operator!=(bench_range_end_t)
    {
        return m_i != m_end;
    }

private:
    int m_i;
    int m_end;
};

template <class T>
auto bench(T&& t)
{
    std::vector<std::chrono::microseconds> expr;

    for (int j = 0; j < 100; ++j) {
        auto begin = tos::high_resolution_clock::now();
        t();
        auto end = tos::high_resolution_clock::now();
        tos::this_thread::yield();
        expr.emplace_back(end - begin);
    }

    return expr;
}

static void esp_main()
{
    using namespace tos;
    using namespace tos::tos_literals;

    auto usart = tos::open(tos::devs::usart<0>, tos::uart::default_9600);

    time_emsha();

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
