#pragma once

#include <tos/detail/coro.hpp>
#include <utility>

namespace tos {
template<class ResultT, class ErrorT = void>
class task {
public:
    struct promise_type {
        task get_return_object() noexcept {
            return {};
        }
        std::suspend_never initial_suspend() noexcept {
            return {};
        }
        std::suspend_never final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
        }

        template<class T>
        void return_value(T&& val) {
            m_val = std::forward<T>(val);
        }

        ResultT m_val;
    };

    struct promise_type;
    using coro_handle = std::coroutine_handle<promise_type>;

    constexpr bool await_ready() const noexcept {
        return false;
    }

    std::coroutine_handle<> await_suspend(std::coroutine_handle<> h) {
        m_cont = h;
        return *this;
    }

    constexpr void await_resume() const noexcept {
    }

private:
    std::coroutine_handle<> m_cont;
};
} // namespace tos