#pragma once

#include <algorithm>
#include <tos/coro/countdown.hpp>
#include <tos/detail/poll.hpp>

namespace tos::coro {
template<class It, class Fn>
auto for_each_n(It begin, int n, const Fn& cb) {
    return coro::countdown::start(n, [begin, &cb](coro::countdown& cd) {
        std::for_each_n(begin, cd.count, [&cb, &cd](auto& elem) {
            [](auto& elem, auto& cd, auto& cb) -> coro::detached {
                co_await cb(elem);
                co_await cd.signal();
            }(elem, cd, cb);
        });
    });
}
} // namespace tos::coro