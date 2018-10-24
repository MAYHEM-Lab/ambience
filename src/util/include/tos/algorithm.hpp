//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/utility.hpp>
#include <tos/span.hpp>
#include "tuple.hpp"

namespace tos
{
    template<class InputIt, class T, class BinaryOperation>
    constexpr T accumulate(InputIt first, InputIt last, T init,
            BinaryOperation op)
    {
        for (; first != last; ++first) {
            init = op(std::move(init), *first); // std::move since C++20
        }
        return init;
    }

    template<class InputIt, class T>
    T accumulate(InputIt first, InputIt last, T init)
    {
        return accumulate(first, last, std::move(init), [](const T& base, const T& e) {
            return base + e;
        });
    }

    inline int atoi(tos::span<const char> chars)
    {
        int res = 0;
        for (auto p = chars.begin(); p != chars.end() && *p != 0; ++p)
        {
            res = res * 10 + *p - '0';
        }
        return res;
    }

    namespace impl
    {
        template <class FuncT, class TupT, size_t... Is>
        auto apply(FuncT&& fun, TupT&& args, std::index_sequence<Is...>)
        {
            using namespace std;
            using tos::get;
            return forward<FuncT>(fun)(get<Is>(forward<TupT>(args))...);
        }
    }

    template <class FuncT, class TupT>
    auto apply(FuncT &&fun, TupT &&args)
    {
        using namespace std;
        using tup_t = remove_reference_t<TupT>;
        constexpr size_t sz = tuple_size<tup_t>::value;
        return impl::apply(forward<FuncT>(fun), forward<TupT>(args), make_index_sequence<sz>());
    }

    template <class FuncT, class... ArgTs>
    auto invoke(FuncT &&fun, ArgTs &&...args)
    {
        using namespace std;
        constexpr size_t sz = sizeof...(ArgTs);
        return impl::apply(
                forward<FuncT>(fun),
                tos::tuple<ArgTs...>(forward<ArgTs>(args)...),
                        make_index_sequence<sz>());
    }
}
