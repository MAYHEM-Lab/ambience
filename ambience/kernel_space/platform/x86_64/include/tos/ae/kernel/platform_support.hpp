#pragma once

#include "on_demand_interrupt.hpp"
#include "timer.hpp"
#include <common/common_timer_multiplexer.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/preemption.hpp>
#include <tos/board.hpp>
#include <tos/x86_64/assembly.hpp>

struct platform_group_args {
    tos::physical_page_allocator* page_alloc;
    tos::interrupt_trampoline* trampoline;
    tos::x86_64::translation_table* table;
};

struct rdtsc_clock : tos::self_pointing<rdtsc_clock> {
    using rep = uint64_t;
    using period = std::micro;
    using duration = std::chrono::duration<rep, period>;
    using time_point = std::chrono::time_point<rdtsc_clock>;

    uint64_t calibrate; // ticks per microsecond

    time_point now() const {
        return time_point{duration{tos::x86_64::rdtsc() / calibrate}};
    }
};

class platform_support {
public:
    void stage1_init() {
    }

    void stage2_init();

    auto init_serial() {
        return tos::bsp::board_spec::default_com::open();
    }

    // Preemption support calls
    auto operator()(tos::preempt_ops::get_timer_t) -> auto& {
        return m_chrono.timer;
    }

    auto operator()(tos::preempt_ops::get_odi_t) -> auto& {
        return m_odi;
    }

    void operator()(tos::preempt_ops::pre_switch_t, tos::kern::tcb&) {
    }

    void operator()(tos::preempt_ops::post_switch_t, tos::kern::tcb&) {
    }

    auto operator()(tos::preempt_ops::set_syscall_handler_t, auto&& arg) {
        tos::x86_64::set_syscall_handler(tos::x86_64::syscall_handler_t(arg));
    }

    auto operator()(tos::preempt_ops::return_to_thread_from_irq_t,
                    tos::any_fiber& from,
                    tos::any_fiber& to) {
        return_from = &from;
        return_to = &to;
        tos::platform::set_post_irq(
            tos::mem_function_ref<&platform_support::do_return>(*this));
    }

    void do_return(tos::x86_64::exception_frame*) {
        tos::platform::reset_post_irq();
        tos::swap_fibers(*return_from, *return_to);
    }

    platform_group_args make_args();

    tos::any_fiber* return_from;
    tos::any_fiber* return_to;

    tos::physical_page_allocator* m_palloc;

    tos::common_timer_multiplex<timer> m_chrono;

    auto& get_chrono() {
        return m_chrono;
    }

    on_demand_interrupt m_odi;
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline =
        tos::make_interrupt_trampoline(m_odi);
    tos::detail::erased_clock<rdtsc_clock> m_rdtsc_clock;
};
