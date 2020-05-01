//
// Created by fatih on 12/23/18.
//

#include <arch/drivers.hpp>
#include <boost/pfr/detail/core17.hpp>
#include <boost/pfr/detail/for_each_field_impl.hpp>
#include <boost/pfr/flat/core.hpp>
#include <common/adxl345.hpp>
#include <common/gpio.hpp>
#include <common/inet/tcp_stream.hpp>
#include <cwpack.hpp>
#include <tos/ft.hpp>
#include <tos/print.hpp>
#include <tos/sync_ring_buf.hpp>

struct vec3i {
    int32_t x, y, z;
};

tos::sync_fixed_fifo<vec3i, 20> fifo;

tos::stack_storage<1024> accel_stack;
auto task = [] {
    using namespace tos::tos_literals;

    auto gpio = tos::open(tos::devs::gpio);

    auto i2c = tos::open(
        tos::devs::i2c<0>, tos::i2c_type::master, gpio, gpio.port.pin5, gpio.port.pin4);

    tos::adxl345 sensor{i2c};
    sensor.powerOn();
    sensor.setRangeSetting(2);

    auto tmr = tos::open(tos::devs::timer<0>);
    auto alarm = tos::open(tos::devs::alarm, tmr);

    while (true) {
        auto [x, y, z] = sensor.read();

        fifo.push({x, y, z});

        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(alarm, 200ms);
    }
};

auto wifi_connect() {
    tos::esp82::wifi w;

conn_:
    auto res = w.connect("cs190b", "cs190bcs190b");
    if (!res)
        goto conn_;

    auto& wconn = force_get(res);

    wconn.wait_for_dhcp();
    lwip_init();

    return std::make_pair(w, std::move(wconn));
}
namespace tos {
namespace msgpack {
template<class PackerT, class T>
void pack(PackerT& p, const T& value);

template<class PackerT>
void pack_one(PackerT& p, int32_t i) {
    p.insert(i);
}

template<class PackerT>
void pack_one(PackerT& p, const std::string& i) {
    p.insert(i);
}

template<class PackerT>
void pack_one(PackerT& p, float i) {
    p.insert(i);
}

template<class PackerT>
void pack_one(PackerT& p, double i) {
    p.insert(i);
}

template<class PackerT, class T>
void pack_one(PackerT& p, const T& t) {
    pack(p, t);
}

template<template<class...> class TupleT, class... Ts, size_t... Is>
void pack_tup(arr_packer& ap, const TupleT<Ts...>& tup, std::index_sequence<Is...>) {
    using std::get;
    (pack_one(ap, get<Is>(tup)), ...);
}

template<class PackerT, class T>
void pack(PackerT& p, const T& value) {
    constexpr std::size_t fields_count_val =
        boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

    auto arr = p.insert_arr(fields_count_val);

    boost::pfr::detail::for_each_field_dispatcher(
        value,
        [&arr](const auto& val) {
            pack_tup(arr, val, std::make_index_sequence<fields_count_val>{});
        },
        boost::pfr::detail::make_index_sequence<fields_count_val>{});
}
} // namespace msgpack
} // namespace tos

template<class StreamT, class ElemT>
void push(StreamT& stream, const char* schema, const ElemT& elem) {
    std::array<uint8_t, 32> buf;
    tos::msgpack::packer p{buf};
    pack(p, elem);
    auto sp = p.get();
    std::copy_n(schema,
                std::min(buf.size() - sp.size(), strlen(schema)),
                buf.begin() + sp.size());
    buf[sp.size() + strlen(schema)] = strlen(schema);
    stream->write(tos::span(buf).slice(0, sp.size() + strlen(schema) + 1));
    tos::this_thread::yield();
}

auto wifi_task = [] {
    auto [w, conn] = wifi_connect();

    while (true) {
        auto try_conn = tos::esp82::connect(conn, {{169, 231, 9, 60}}, {9993});
        if (!try_conn)
            continue;

        auto& conn = force_get(try_conn);
        tos::tcp_stream<tos::esp82::tcp_endpoint> stream{std::move(conn)};

        auto elem = fifo.pop();
        push(stream, "bakir/adxl", elem);
    }
};

void tos_main() {
    tos::launch(accel_stack, task);
    tos::launch(tos::alloc_stack, wifi_task);
}
