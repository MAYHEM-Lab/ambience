#pragma once

#include <tos/detail/coro.hpp>
#include <tos/expected.hpp>
#include <tos/task.hpp>
#include <utility>

namespace tos::coro {
struct detached {
    struct promise_type {
        detached get_return_object() noexcept {
            return detached{};
        }

        std::suspend_never initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() const noexcept {
            return {};
        }

        void unhandled_exception() {
        }

        void return_void() {
        }
    };
};

class pollable {
public:
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        pollable get_return_object() noexcept {
            return pollable{coro_handle::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_always final_suspend() const noexcept {
            return {};
        }

        void unhandled_exception() {
        }

        void return_void() {
        }
    };

    explicit pollable(coro_handle handle)
        : handle_(handle) {
    }

    pollable(pollable&& rhs) noexcept
        : handle_(std::exchange(rhs.handle_, coro_handle{})) {
    }

    bool done() const {
        return handle_.done();
    }

    bool run() {
        if (!handle_.done()) {
            handle_.resume();
        }
        return handle_.done();
    }

    ~pollable() {
        if (handle_) {
            handle_.destroy();
        }
    }

private:
    coro_handle handle_;
};

template<class T, class BeforeFin = tos::ignore_t>
pollable make_pollable(Task<T> x, BeforeFin fin = {}) {
    if constexpr (std::is_invocable_v<BeforeFin>) {
        co_await x;
        fin();
    } else {
        fin(co_await x);
    }
    co_return;
}

template<class AwaitableT, class BeforeFin = tos::ignore_t>
detached make_detached(AwaitableT x, BeforeFin fin = {}) {
    if constexpr (std::is_invocable_v<BeforeFin>) {
        if constexpr (std::is_invocable_v<AwaitableT>) {
            co_await x();
        } else {
            co_await x;
        }
        fin();
    } else {
        if constexpr (std::is_invocable_v<AwaitableT>) {
            fin(co_await x());
        } else {
            fin(co_await x);
        }
    }
    co_return;
}
} // namespace tos::coro