#pragma once

#include <tos/detail/coro.hpp>

namespace tos::coro {
struct countdown {
    explicit countdown(int count)
        : count{count} {
    }
    countdown(const countdown&) = delete;
    countdown(countdown&&) = delete;

    template<class Fn>
    static auto start(int n, Fn&& fn) {
        struct awaiter {
            bool await_ready() const {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle) {
                cd.m_handle = handle;
                (fn)(cd);
            }

            void await_resume() {
                cd.m_handle.resume();
            }

            awaiter(int n, Fn&& fn)
                : cd{n}
                , fn{std::forward<Fn>(fn)} {
            }

            countdown cd;
            Fn fn;
        };

        return awaiter{n, std::forward<Fn>(fn)};
    }

    template<class Fn>
    auto start(const Fn& fn) {
        struct awaiter {
            bool await_ready() const {
                return false;
            }

            void await_suspend(std::coroutine_handle<> handle) {
                cd->m_handle = handle;
                (*fn)();
            }

            void await_resume() {
                cd->m_handle.resume();
            }

            const Fn* fn;
            countdown* cd;
        };

        return awaiter{.fn = &fn, .cd = this};
    }

    auto signal() {
        struct awaiter {
            // When this returns false, await_suspend is called.
            // We want it to be called when cd->count == 0
            bool await_ready() const {
                return cd->count != 0;
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> handle) {
                return std::exchange(cd->m_handle, handle);
            }

            void await_resume() {
            }

            countdown* cd;
        };

        --count;
        return awaiter{.cd = this};
    }

    int count;
    std::coroutine_handle<> m_handle;
};
} // namespace tos::coro