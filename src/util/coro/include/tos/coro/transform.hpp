#pragma once

#include <algorithm>
#include <tos/algorithm.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/detail/poll.hpp>

namespace tos::coro {
template<class It, class Out, class Fn>
auto transform_n(It begin, int n, Out out, const Fn& cb) {
    return coro::countdown::start(n, [begin, &cb, out](coro::countdown& cd) mutable {
        auto n = cd.count;
        for (auto i = 0; i < n; ++i, ++begin, ++out) {
            [](auto& elem, auto& cd, auto& cb, auto& res) -> coro::detached {
                res = co_await cb(std::forward<decltype(elem)>(elem));
                co_await cd.signal();
            }(*begin, cd, cb, *out);
        }
    });
}
} // namespace tos::coro