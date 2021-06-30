#pragma once

#include "on_demand_interrupt.hpp"
#include <arch/drivers.hpp>
#include <common/common_timer_multiplexer.hpp>
#include <common/timer.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/aarch64/semihosting.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/preemption.hpp>

struct platform_group_args {
    tos::physical_page_allocator* page_alloc;
    tos::interrupt_trampoline* trampoline;
    tos::aarch64::translation_table* table;
};

class semihosting_output : public tos::self_pointing<semihosting_output> {
public:
    int write(tos::span<const uint8_t> data) {
        auto res = data.size();
        while (!data.empty()) {
            auto len = std::min(data.size(), m_buf.size() - 1);
            auto it = std::copy_n(data.begin(), len, m_buf.begin());
            *it = 0;
            tos::aarch64::semihosting::write0(m_buf.data());
            data = data.slice(len);
        }
        return res;
    }

private:
    std::array<char, 128> m_buf;
};


struct platform_support {
    void stage1_init();
    void stage2_init();

    //    semihosting_output init_serial();
    tos::raspi3::uart0 init_serial();

    // Preemption support calls
    auto operator()(tos::preempt_ops::get_timer_t) -> auto& {
        return m_chrono.get().timer;
    }

    auto operator()(tos::preempt_ops::get_odi_t) -> auto& {
        return m_odi;
    }

    auto operator()(tos::preempt_ops::set_syscall_handler_t, auto&& arg) {
        tos::aarch64::exception::set_svc_handler(
            tos::aarch64::exception::svc_handler_t(arg));
    }

    auto operator()(tos::preempt_ops::return_to_thread_from_irq_t,
                    tos::kern::tcb& from,
                    tos::kern::tcb& to) {
        return_from = &from;
        return_to = &to;
        ic.set_post_irq(tos::mem_function_ref<&platform_support::do_return>(*this));
    }

    platform_group_args make_args();

    void do_return() {
        ic.reset_post_irq();
        tos::swap_context(*return_from, *return_to, tos::int_ctx{});
    }

    tos::raspi3::interrupt_controller ic;

    tos::kern::tcb* return_from;
    tos::kern::tcb* return_to;

    tos::physical_page_allocator* m_palloc;

    tos::late_constructed<tos::common_timer_multiplex<tos::raspi3::system_timer>>
        m_chrono;

    auto& get_chrono() {
        return m_chrono.get();
    }

    svc_on_demand_interrupt m_odi;
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline =
        tos::make_interrupt_trampoline(m_odi);
};