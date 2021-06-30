#pragma once

#include "on_demand_interrupt.hpp"
#include <common/common_timer_multiplexer.hpp>
#include <tos/arch.hpp>
#include <tos/board.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/preemption.hpp>

struct platform_group_args {
    tos::interrupt_trampoline* trampoline;
};

struct platform_support {
    void stage1_init();
    void stage2_init();

    //    semihosting_output init_serial();
    auto init_serial() {
        return tos::bsp::board_spec::default_com::open();
    }

    // Preemption support calls
    auto operator()(tos::preempt_ops::get_timer_t) -> auto& {
        return preempt_timer;
    }

    auto operator()(tos::preempt_ops::get_odi_t) -> auto& {
        return m_odi;
    }

    auto operator()(tos::preempt_ops::set_syscall_handler_t, auto&& arg) {
        tos::arm::exception::set_svc_handler(tos::arm::exception::svc_handler_t(arg));
    }

    auto operator()(tos::preempt_ops::return_to_thread_from_irq_t,
                    tos::kern::tcb& from,
                    tos::kern::tcb& to) {
        tos::swap_context(from, to, tos::int_ctx{});
    }

    platform_group_args make_args();

    tos::stm32::general_timer preempt_timer{tos::stm32::detail::gen_timers[1]};
    tos::common_timer_multiplex<tos::stm32::general_timer> m_chrono{
        tos::stm32::detail::gen_timers[0]};

    auto& get_chrono() {
        return m_chrono;
    }
    svc_on_demand_interrupt m_odi;
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline =
        tos::make_interrupt_trampoline(m_odi);
};
