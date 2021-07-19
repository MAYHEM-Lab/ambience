#pragma once

#include <tos/coro/countdown.hpp>
#include <tos/coro/meta.hpp>
#include <tos/detail/poll.hpp>

namespace tos::coro {
template<class... Awaitables>
using when_all_result_t = std::tuple<meta::await_result_t<Awaitables>...>;

namespace detail {
template<class... Ts, std::size_t... Is>
auto convert_from_late_constructed(std::index_sequence<Is...>,
                                   std::tuple<late_constructed<Ts>...>&& from)
    -> std::tuple<Ts...> {
    return std::tuple<Ts...>{std::forward<Ts>(std::get<Is>(from).get())...};
}

template<class... Awaitables, std::size_t... Is>
auto when_all_impl(std::index_sequence<Is...> is, Awaitables&&... awaitables)
    -> tos::Task<when_all_result_t<Awaitables...>> {
    std::tuple<late_constructed<meta::await_result_t<Awaitables>>...> res;

    countdown cd(sizeof...(Awaitables));

    co_await cd.start([&awaitables..., &cd, &res] {
        (([](auto& awaitable, auto& cd, auto& res) -> detached {
             std::get<Is>(res).emplace(co_await awaitable);
             co_await cd.signal();
         }(awaitables, cd, res)),
         ...);
    });

    co_return convert_from_late_constructed(is, std::move(res));
}

template<class... Awaitables, std::size_t... Is>
auto when_all_bad(std::index_sequence<Is...> is, Awaitables&&... awaitables)
    -> tos::Task<when_all_result_t<Awaitables...>> {
    co_return when_all_result_t<Awaitables...>{(co_await awaitables)...};
}
} // namespace detail

template<class... Awaitables>
auto when_all(Awaitables&&... awaitables) {
    return detail::when_all_impl(std::make_index_sequence<sizeof...(Awaitables)>(),
                                 std::forward<Awaitables>(awaitables)...);
}
} // namespace tos::coro