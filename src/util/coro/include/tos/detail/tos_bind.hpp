#pragma once

#include <tos/detail/poll.hpp>
#include <tos/job.hpp>
#include <tos/scheduler.hpp>

namespace tos {
class coro_job : public job {
public:
    coro_job(context& ctx, coro::pollable&& p)
        : job(ctx)
        , m_pollable{std::move(p)} {
    }

    void operator()() override {
        m_pollable.run();
    }

private:
    coro::pollable m_pollable;
};

namespace coro {
inline auto yield() {
    struct awaiter
        : tos::job
        , tos::int_guard {
        using job::job;

        bool await_ready() const noexcept {
            return false;
        }

        void await_suspend(std::coroutine_handle<> coro) {
            m_coro = coro;
            tos::kern::make_runnable(*this);
        }

        void await_resume() {
        }

        void operator()() override {
            m_coro.resume();
        }

        std::coroutine_handle<> m_coro;
    };

    return awaiter{tos::current_context()};
}
} // namespace coro
} // namespace tos