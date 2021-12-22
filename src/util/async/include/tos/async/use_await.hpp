#pragma once

#include <tos/detail/coro.hpp>
#include <tos/function_ref.hpp>

namespace tos::async {
struct use_await {
    template<class Start>
    auto get_completion(Start&& start, function_ref<void()>& cb) {
        struct awaiter {
            bool await_ready() const noexcept {
                return false;
            }

            void await_suspend(std::coroutine_handle<> resume) noexcept {
                *m_cb = coro_resumer(resume);
                m_fn();
            }

            auto await_resume() const noexcept {
            }

            function_ref<void()>* m_cb;
            std::remove_reference<Start> m_fn;
        };

        return awaiter{.m_cb = cb, .m_fn = std::forward<Start>(start)};
    }
};
} // namespace tos::async