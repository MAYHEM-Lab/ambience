#pragma once

#include <tos/cancellation_token.hpp>
#include <tos/detail/coro.hpp>
#include <tos/fiber.hpp>
#include <tos/interrupt.hpp>
#include <tos/core_waitable.hpp>

namespace tos {
struct waitable : core_waitable {
    /**
     * Makes the current thread yield and block on this
     * waitable object.
     *
     * Interrupts must be disabled when this function
     * is called. Interrupts will be re-enabled by the
     * scheduler after yielding.
     *
     * If this function is called from a non-task
     * context, the behaviour is undefined.
     */
    void wait(const int_guard&);

    bool wait(const int_guard&, cancellation_token& cancel);

    template<Fiber FibT>
    void wait(FibT& fib, const int_guard&);

    auto operator co_await() {
        struct awaiter : job {
            awaiter(context& ctx, waitable& w)
                : job(ctx)
                , m_waitable{&w} {
            }

            bool await_ready() const noexcept {
                return false;
            }

            void await_suspend(std::coroutine_handle<> coro) {
                m_cont = coro;
                m_waitable->add(*this);
            }

            void await_resume() {
            }

            void operator()() override {
                m_cont.resume();
            }

            awaiter(awaiter&&) = delete;
            awaiter(const awaiter&) = delete;

            waitable* m_waitable;
            std::coroutine_handle<> m_cont;
        };

        return awaiter{current_context(), *this};
    }
};

template<Fiber FibT>
void waitable::wait(FibT& fib, const int_guard&) {
    struct inline_job : job {
        inline_job(FibT& fib)
            : job(current_context())
            , m_fib(&fib) {
        }

        void operator()() override {
            m_fib->resume();
        }

        FibT* m_fib;
    };

    inline_job j{fib};
    fib.suspend([&] { add(j); });
}
} // namespace tos
