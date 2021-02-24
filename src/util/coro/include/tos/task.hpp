#pragma once

#include <iostream>
#include <tos/detail/coro.hpp>
#include <utility>

#define __PRETTY_FUNCTION__ __FUNCSIG__
#define dump() std::cerr << this << " " << __PRETTY_FUNCTION__ << '\n';

namespace tos {
template<class ResultT, class ErrorT = void>
class task {
public:
    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    struct promise_type {
        task get_return_object() noexcept {
            dump();
            return task{coro_handle::from_promise(*this)};
        }

        std::suspend_never initial_suspend() noexcept {
            dump();
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            dump();

            return {};
        }

        void unhandled_exception() {
        }

        template<class T>
        void return_value(T&& val) {
            dump();

            m_val = std::forward<T>(val);
        }

        ResultT m_val;
    };

    task(coro_handle handle)
        : handle_(handle) {
        dump();
    }
    task(task&) = delete;
    task(task&&) = delete;

    constexpr bool await_ready() const noexcept {
        dump();

        return false;
    }

    void await_suspend(std::coroutine_handle<> h) {
        dump();

        m_cont = h;
    }

    constexpr void await_resume() const noexcept {
        dump();
    }

private:
    std::coroutine_handle<> m_cont;
    coro_handle handle_;
};
} // namespace tos