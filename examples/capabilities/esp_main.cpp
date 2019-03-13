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
    caps::emsha::signer s{"0123456789abcdef"};

    auto cap = caps::mkcaps({
        authn::cap_t{ authn::id_t{"foo"}, authn::rights::full },
        authn::cap_t{ authn::path_t{"bar.txt"}, authn::rights::full }
    }, s);

    caps::attach(*cap, *caps::mkcaps({
        authn::cap_t{ authn::id_t{"foo"}, authn::rights::read },
        authn::cap_t{ authn::path_t{"bar.txt"}, authn::rights::full }
    }), s);

    caps::attach(*cap, *caps::mkcaps({
        authn::cap_t{ authn::id_t{"foo"}, authn::rights::read },
        authn::cap_t{ authn::path_t{"bar.txt"}, authn::rights::read }
    }), s);
    for (int j = 0; j < 30; ++j)
    {
        auto begin = tos::high_resolution_clock::now();
        for (int i = 0; i < 100; ++i)
        {
            caps::verify(*cap, s, 0, {});
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

struct bench_it
{
    constexpr bool operator==(bench_range_end_t)
    {
        return m_i == m_end;
    }

    constexpr bool operator!=(bench_range_end_t)
    {
        return m_i != m_end;
    }

    constexpr int operator*()
    {
        return m_i;
    }

    constexpr bench_it& operator++()
    {
        ++m_i;
        return *this;
    }

    constexpr bench_it operator++(int)
    {
        auto cp = *this;
        ++*this;
        return cp;
    }

    explicit constexpr bench_it(int end) : m_end{end} {}

private:
    int m_i = 0;
    int m_end;
};

struct bench_range
{
    explicit constexpr bench_range(int len) : m_len{len} {}

    constexpr bench_it begin() {
        return bench_it{m_len};
    }

    constexpr bench_range_end_t end() { return {}; };

private:
    int m_len;
};

constexpr int foo(int x)
{
    int sum = 0;
    for (auto _ : bench_range{x})
    {
        sum += _;
    }
    return sum;
}

static_assert(foo(5) == 10);

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
    tos::launch(tos::stack_size_t{8096}, esp_main);
}
