#pragma once

#include <functional>
#include <tos/ft.hpp>
#include <tos/function_ref.hpp>
#include <tos/self_pointing.hpp>

namespace tos {
namespace preempt_ops {
struct return_to_thread_from_irq_t {};
constexpr return_to_thread_from_irq_t return_to_thread_from_irq{};

struct set_syscall_handler_t {};
constexpr set_syscall_handler_t set_syscall_handler{};

struct get_timer_t {};
constexpr get_timer_t get_timer{};

struct get_odi_t {};
constexpr get_odi_t get_odi{};

struct pre_switch_t {};
constexpr pre_switch_t pre_switch{};

struct post_switch_t {};
constexpr post_switch_t post_switch{};
} // namespace preempt_ops

template<class T>
concept PreemptionContext = requires(T t) {
    t(preempt_ops::get_timer);
    t(preempt_ops::get_odi);
};

template<class T>
concept PrePostHandler = requires(T t) {
    t(preempt_ops::pre_switch, std::declval<kern::tcb&>());
    t(preempt_ops::post_switch, std::declval<kern::tcb&>());
};

namespace global {
    inline function_ref<bool()> user_on_error([](void*) { return false; });
}

enum class user_reason
{
    voluntary,
    preempt,
    error,
};

template<PreemptionContext PreemptContext>
struct preempter {
    void setup(PreemptContext& ctx, int ticks) {
        m_ctx = &ctx;
        auto& tmr = timer();

        tmr.set_callback(tos::mem_function_ref<&preempter::preempt>(*this));
        tmr.set_frequency(1000 / ticks);
        global::user_on_error = mem_function_ref<&preempter::error>(*this);
    }

    bool error() {
        timer().disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx()(preempt_ops::post_switch, *m_thread);
        }

        m_exit_reason = user_reason::error;
        ctx()(preempt_ops::return_to_thread_from_irq, *m_thread, *m_self);
        
        return true;
    }

    void preempt() {
        timer().disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx()(preempt_ops::post_switch, *m_thread);
        }

        m_exit_reason = user_reason::preempt;
        ctx()(preempt_ops::return_to_thread_from_irq, *m_thread, *m_self);
    }

    void syshandler() {
        timer().disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx()(preempt_ops::post_switch, *m_thread);
        }

        m_exit_reason = user_reason::voluntary;
        tos::swap_context(*m_thread, *m_self, tos::int_ctx{});
    }

    user_reason run(kern::tcb& thread) {
        m_self = tos::self();
        m_thread = &thread;
        ctx()(preempt_ops::get_odi)([this](auto&&...) {
            auto syscall = [this](auto&&...) { this->syshandler(); };
            ctx()(preempt_ops::set_syscall_handler, syscall);

            if constexpr (PrePostHandler<PreemptContext>) {
                ctx()(preempt_ops::pre_switch, *m_thread);
            }

            timer().enable();

            tos::swap_context(*m_self, *m_thread, tos::int_ctx{});
        });
        return m_exit_reason;
    }

    auto& timer() {
        return meta::deref(ctx()(preempt_ops::get_timer));
    }

    auto& ctx() {
        return *m_ctx;
    }

    user_reason m_exit_reason;
    kern::tcb* m_thread;
    kern::tcb* m_self;
    PreemptContext* m_ctx;
};

template<PreemptionContext PreemptContext>
bool run_preemptively_for(PreemptContext& ctx, kern::tcb& thread) {
    auto& self = *tos::self();
    auto& tmr = meta::deref(ctx(preempt_ops::get_timer));

    bool preempted = false;
    auto preempt = [&] {
        tmr.disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx(preempt_ops::post_switch, thread);
        }

        preempted = true;
        ctx(preempt_ops::return_to_thread_from_irq, thread, self);
    };

    tmr.set_callback(tos::function_ref<void()>(preempt));
    tmr.set_frequency(1000 / 5);

    auto syshandler = [&](auto&&...) {
        tmr.disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx(preempt_ops::post_switch, thread);
        }

        tos::swap_context(thread, self, tos::int_ctx{});
    };

    ctx(preempt_ops::get_odi)([&](auto&&...) {
        ctx(preempt_ops::set_syscall_handler, syshandler);

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx(preempt_ops::pre_switch, thread);
        }

        tmr.enable();

        tos::swap_context(self, thread, tos::int_ctx{});
    });

    return preempted;
}

template<PreemptionContext Ctx>
function_ref<bool(kern::tcb&)> make_erased_preemptive_runner(Ctx& ctx) {
    return function_ref<bool(kern::tcb&)>(
        [](kern::tcb& thread, void* arg) -> bool {
            return run_preemptively_for(*static_cast<Ctx*>(arg), thread);
        },
        &ctx);
}
} // namespace tos
