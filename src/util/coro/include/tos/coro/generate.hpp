#pragma once

#include <algorithm>
#include <tos/algorithm.hpp>
#include <tos/coro/countdown.hpp>
#include <tos/detail/poll.hpp>

namespace tos::coro {
template<class Out, class Fn>
auto generate_n(int n, Out out, const Fn& cb) {
    return coro::countdown::start(n, [&cb, out](coro::countdown& cd) mutable {
        auto n = cd.count;
        for (auto i = 0; i < n; ++i, ++out) {
            [](int el, auto& cd, auto& cb, auto& res) -> coro::detached {
                res = co_await cb(el);
                co_await cd.signal();
            }(i, cd, cb, *out);
        }
    });
}
} // namespace tos::coro