//
// Created by Mehmet Fatih BAKIR on 10/05/2018.
//

#pragma once

#include <tos/utility.hpp>
#include <tos/span.hpp>
#include "tuple.hpp"

namespace tos
{
    namespace std
    {
        template <class T>
        constexpr T max(const T &a, const T &b) {
            return a < b ? b : a;
        }

        template <class T>
        constexpr T min(const T &a, const T &b) {
            return a > b ? b : a;
        }
    }

    template <class ItT>
    constexpr auto max_range(ItT begin, ItT end) -> std::remove_reference_t<decltype(*begin)>
    {
        if (begin + 1 == end)
        {
            return *begin;
        }
        else
        {
            auto mid = begin + (end - begin) / 2;
            return std::max(
                max_range(begin, mid),
                max_range(mid, end)
            );
        }
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
        auto apply(FuncT&& fun, TupT&& args, tos::std::index_sequence<Is...>)
        {
            using namespace tos::std;
            using tos::std::get;
            return forward<FuncT>(fun)(get<Is>(forward<TupT>(args))...);
        }
    }

    template <class FuncT, class TupT>
    auto apply(FuncT &&fun, TupT &&args)
    {
        using namespace tos::std;
        using tup_t = remove_reference_t<TupT>;
        constexpr size_t sz = tuple_size<tup_t>::value;
        return impl::apply(forward<FuncT>(fun), forward<TupT>(args), make_index_sequence<sz>());
    }

    template <class FuncT, class... ArgTs>
    auto invoke(FuncT &&fun, ArgTs &&...args)
    {
        using namespace tos::std;
        constexpr size_t sz = sizeof...(ArgTs);
        return impl::apply(
                forward<FuncT>(fun),
                tos::std::tuple<ArgTs...>(forward<ArgTs>(args)...),
                        make_index_sequence<sz>());
    }

    namespace std
    {
        template<class ForwardIt, class T >
        void fill(ForwardIt first, ForwardIt last, const T& value)
        {
            for (; first != last; ++first) {
                *first = value;
            }
        }

        template<class InputIt, class OutputIt>
        OutputIt copy(InputIt first, InputIt last,
                OutputIt d_first)
        {
            while (first != last) {
                *d_first++ = *first++;
            }
            return d_first;
        }
    }
}
