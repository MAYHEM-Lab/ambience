#pragma once

#include <type_traits>

namespace tos::coro::meta {
template<class Awaitable>
using await_result_t =
    decltype(std::declval<Awaitable&>().operator co_await().await_resume());
}