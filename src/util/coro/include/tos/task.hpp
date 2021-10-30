#pragma once

#include <cassert>
#include <tos/detail/coro.hpp>
#include <tos/late_constructed.hpp>
#include <tos/result.hpp>
#include <utility>

namespace tos {
template<class T>
class TaskPromiseBase;

template<class T, bool = std::is_reference_v<T>>
class TaskPromise;

template<typename T = void>
class Task {
public:
    Task() = default;
    Task(Task& t) = delete;

    Task(Task&& t) noexcept
        : m_handle(std::exchange(t.m_handle, {})) {
    }

    ~Task() {
        if (m_handle) {
            m_handle.destroy();
        }
    }

    using promise_type = TaskPromise<T>;
    using promise_coro_handle = std::coroutine_handle<promise_type>;

    auto operator co_await() const {
        struct awaiter {
            awaiter(promise_coro_handle coro)
                : m_self_handle(coro) {
            }

            bool await_ready() const noexcept {
                // Since we initial_suspend, we cannot be ready by definition.
                return false;
            }

            std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
                m_self_handle.promise().continuation = continuation;
                // Somebody co_await'ed us, so we resume ourselves by returning our
                // handle.
                return m_self_handle;
            }

            decltype(auto) await_resume() {
                if constexpr (!std::is_void_v<T>) {
                    return std::move(m_self_handle.promise().result());
                }
            }

        private:
            promise_coro_handle m_self_handle;
        };

        return awaiter{m_handle};
    }

private:
    explicit Task(promise_coro_handle coro)
        : m_handle(coro) {
    }

    friend class TaskPromiseBase<T>;
    friend class TaskPromise<T>;
    promise_coro_handle m_handle{};
};

template<class T>
class TaskPromiseBase {
protected:
    using promise_type = TaskPromise<T>;
    using promise_coro_handle = std::coroutine_handle<promise_type>;

    friend class Task<T>;
    std::coroutine_handle<> continuation;

public:
    std::suspend_always initial_suspend() const noexcept {
        return {};
    }

    auto final_suspend() const noexcept {
        struct awaiter {
            bool await_ready() const noexcept {
                return false;
            }

            std::coroutine_handle<>
            await_suspend(promise_coro_handle coroHandle) noexcept {
                // final_suspend is called by the coroutine itself, so we know the
                // given handle refers to ourselves, and since this is final_suspend,
                // we will resume whoever was waiting on us.
                return coroHandle.promise().continuation;
            }

            void await_resume() noexcept {
            }
        };

        return awaiter{};
    }

    void unhandled_exception() {
    }

    Task<T> get_return_object() noexcept {
        return Task<T>{
            promise_coro_handle::from_promise(static_cast<promise_type&>(*this))};
    }
};


template<class T>
class TaskPromise<T, false> : public TaskPromiseBase<T> {
    tos::late_constructed<T> m_value;

public:
    template<class... ValT>
    void return_value(ValT&&... value) {
        m_value.emplace(std::forward<ValT>(value)...);
    }

    T& result() {
        return m_value.get();
    }
};

template<class T>
class TaskPromise<T, true> : public TaskPromiseBase<T> {
    std::remove_reference_t<T>* m_value;

public:
    template<class... ValT>
    void return_value(T& value) {
        m_value = &value;
    }

    T& result() {
        return *m_value;
    }
};

template<>
class TaskPromise<void, false> : public TaskPromiseBase<void> {
public:
    void return_void() {
    }
};

template<class T>
using async = Task<result<T>>;
} // namespace tos