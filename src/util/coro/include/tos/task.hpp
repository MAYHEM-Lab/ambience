#pragma once

#include <cassert>
#include <tos/detail/coro.hpp>
#include <utility>

namespace tos {
template<typename T = void>
class Task {
    class TaskPromiseBase;
    class TaskPromise;

public:
    Task()
        : coroHandle(nullptr) {
    }
    Task(Task& t) = delete;

    Task(Task&& t) noexcept
        : coroHandle(std::exchange(t.coroHandle, {})) {
    }

    ~Task() {
        if (this->coroHandle) {
            this->coroHandle.destroy();
        }
    }

    using promise_type = TaskPromise;
    using promise_coro_handle = std::coroutine_handle<promise_type>;

    auto operator co_await() const& {
        struct awaiter {
            awaiter(promise_coro_handle coro)
                : coro(coro) {
            }

            bool await_ready() const noexcept {
                return false;
            }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
                this->coro.promise().continuation = continuation;

                return this->coro;
            }

            decltype(auto) await_resume() {
                if constexpr (!std::is_void_v<decltype(this->coro.promise().result())>) {
                    return std::move(this->coro.promise().result());
                } else {
                    this->coro.promise().result();
                }
            }

        private:
            promise_coro_handle coro;
        };

        return awaiter{this->coroHandle};
    }

    auto operator co_await() const&& {
        struct awaiter {
            awaiter(promise_coro_handle coro)
                : coro(coro) {
            }

            bool await_ready() const noexcept {
                return false;
            }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> continuation) {
                this->coro.promise().continuation = continuation;

                return this->coro;
            }

            decltype(auto) await_resume() {
                if constexpr (!std::is_void_v<decltype(this->coro.promise().result())>) {
                    return std::move(this->coro.promise().result());
                } else {
                    this->coro.promise().result();
                }
            }

        private:
            promise_coro_handle coro;
        };

        return awaiter{this->coroHandle};
    }

    bool run() {
        if (!coroHandle.done()) {
            coroHandle.resume();
        }
        return coroHandle.done();
    }

    T value() {
        return coroHandle.promise().result();
    }

private:
    explicit Task(promise_coro_handle coro)
        : coroHandle(coro) {
    }

    promise_coro_handle coroHandle;

    class TaskPromiseBase {
    protected:
        TaskPromiseBase() = default;
        virtual ~TaskPromiseBase() = default;

        friend class Task;
        std::coroutine_handle<> continuation;

    public:
        std::suspend_always initial_suspend() const noexcept {
            return {};
        }

        auto final_suspend() const noexcept {
            struct FinalAwaiter {
                bool await_ready() const noexcept {
                    return false;
                }

                std::coroutine_handle<>
                await_suspend(promise_coro_handle coroHandle) noexcept {
                    return coroHandle.promise().continuation;
                }

                void await_resume() noexcept {
                }
            };

            return FinalAwaiter{};
        }
        void unhandled_exception() {
        }
    };

    class TaskPromise : public TaskPromiseBase {
        T value;

    public:
        void return_value(T value) {
            this->value = value;
        }

        T&& result() {
            return std::move(this->value);
        }

        Task get_return_object() noexcept {
            return Task{promise_coro_handle::from_promise(*this)};
        }
    };
};

template<>
class Task<void>::TaskPromise : public TaskPromiseBase {
public:
    void return_void() {
    }

    void result() {
    }

    Task get_return_object() noexcept {
        return Task{promise_coro_handle::from_promise(*this)};
    }
};

} // namespace tos