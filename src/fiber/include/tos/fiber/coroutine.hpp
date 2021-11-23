#pragma once

#include "tos/detail/poll.hpp"
#include <tos/fiber/this_fiber.hpp>

namespace tos::fiber {
template<class CallableT, class... Args>
auto co_adapt(CallableT&& callable) {
    struct awaiter {
        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> cont) {
            registered_owning::start(stack_size_t{TOS_DEFAULT_STACK_SIZE},
                                     [this, cont]([[maybe_unused]] auto& fib) mutable {
                                         m_ret = m_callable();
                                         cont.resume();
                                     });
        }

        using ret_type = decltype(callable());
        ret_type await_resume() noexcept {
            return m_ret;
        }

        awaiter(auto&& callable)
            : m_callable{std::forward<decltype(callable)>(callable)} {
        }

        ret_type m_ret;
        decltype(callable) m_callable;
    };

    return awaiter(std::forward<CallableT>(callable));
}

template<class CallableT, class... Args>
auto co_adapt(CallableT&& callable, Args&&... args) {
    auto fn = [&] {
        return std::forward<CallableT>(callable)(std::forward<Args>(args)...);
    };
    return co_adapt(fn);
}

template<class Awaitable, Fiber FibT>
auto fiber_await(FibT& fiber, Awaitable&& awaitable) {
    bool suspended = false;

    decltype(awaitable.operator co_await()) res;

    auto x = tos::coro::make_pollable(std::forward<Awaitable>(awaitable), [&](auto&& val) {
        // We are either completing synchronously, or the fiber is suspended.
        res = std::move(val);
        if (suspended) {
            fiber.resume();
        }
    });

    if (!x.run()) {
        suspended = true;
        fiber.suspend();
    }

    if (suspended) {
        // Destroy
        x.run();
    }

    return res;
}
} // namespace tos::fiber