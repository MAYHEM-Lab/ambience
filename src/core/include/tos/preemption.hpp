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

template<PreemptionContext PreemptContext>
bool run_preemptively_for(PreemptContext& ctx, kern::tcb& thread, int ticks) {
    auto& self = *tos::self();
    auto& tmr = meta::deref(ctx(preempt_ops::get_timer));
    auto& odi = ctx(preempt_ops::get_odi);

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
    tmr.set_frequency(1000 / ticks);

    auto syshandler = [&](auto&&...) {
        tmr.disable();

        if constexpr (PrePostHandler<PreemptContext>) {
            ctx(preempt_ops::post_switch, thread);
        }

        tos::swap_context(thread, self, tos::int_ctx{});
    };

    odi([&](auto&&...) {
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
auto make_preemptive_runner(Ctx& ctx) {
    return [&ctx](kern::tcb& thread, int ticks) -> bool {
        return run_preemptively_for(ctx, thread, ticks);
    };
}

template<PreemptionContext Ctx>
function_ref<bool(kern::tcb&, int)> make_erased_preemptive_runner(Ctx& ctx) {
    return function_ref<bool(kern::tcb&, int)>(
        [](kern::tcb& thread, int ticks, void* arg) -> bool {
            return run_preemptively_for(*static_cast<Ctx*>(arg), thread, ticks);
        },
        &ctx);
}
} // namespace tos
