#pragma once

#if __has_include(<experimental/coroutine>)
#include <experimental/coroutine>

namespace std {
using experimental::coroutine_handle;
using experimental::suspend_always;
using experimental::suspend_never;
} // namespace std
#elif __has_include(<coroutine>)
#include <coroutine>
#endif