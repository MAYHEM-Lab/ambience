//
// Created by fatih on 4/12/19.
//

#include <iostream>
#include <cwpack.hpp>
#include <boost/pfr.hpp>

namespace tos
{
namespace msgpack
{
    template <class PackerT, class T>
    void pack(PackerT& p, const T& value);

    template <class PackerT>
    void pack_one(PackerT& p, int32_t i) { p.insert(i); }

    template <class PackerT>
    void pack_one(PackerT& p, const std::string& i) { p.insert(i); }

    template <class PackerT>
    void pack_one(PackerT& p, float i) { p.insert(i); }

    template <class PackerT>
    void pack_one(PackerT& p, double i) { p.insert(i); }

    template <class PackerT, class T>
    void pack_one(PackerT& p, const T& t)
    {
        pack(p, t);
    }

    template <template <class...> class TupleT, class... Ts, size_t... Is>
    void pack_tup(arr_packer& ap, const TupleT<Ts...>& tup, std::index_sequence<Is...>)
    {
        using std::get;
        (pack_one(ap, get<Is>(tup)), ...);
    }

    template <class PackerT, class T>
    void pack(PackerT& p, const T& value) {
        constexpr std::size_t fields_count_val = boost::pfr::detail::fields_count<std::remove_reference_t<T>>();

        auto arr = p.insert_arr(fields_count_val);

        boost::pfr::detail::for_each_field_dispatcher(
                value,
                [&arr](const auto& val) {
                    pack_tup(arr, val, std::make_index_sequence<fields_count_val>{});
                },
                boost::pfr::detail::make_index_sequence<fields_count_val>{}
        );
    }
}
}

struct v3
{
    float x, y, z;
};

struct foo_t
{
    int32_t x;
    float y;
    std::string z;
    v3 a;
};

int main()
{
    char buf[512];
    tos::msgpack::packer p(buf);
    foo_t f { 42, 3.14, "hello", { 1, 2, 3 } };
    pack(p, f);

    for (char c : p.get())
    {
        using namespace std;
        cout << hex << setfill('0') << setw(2) << int16_t(c) << " ";
    }
}