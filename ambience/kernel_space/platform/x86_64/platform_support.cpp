#include "private.hpp"
#include <lwip/timeouts.h>
#include <tos/ae/kernel/platform_support.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>
#include <tos/lwip/common.hpp>
#include <tos/lwip/lwip.hpp>

tos::physical_page_allocator* g_palloc;
tos::ae::registry_base& get_registry();

template<class TimerT>
uint64_t calibrate_tsc(TimerT& timer) {
    uint64_t end;
    tos::semaphore sem{0};
    auto handler = [&] {
        end = tos::x86_64::rdtsc();
        timer.disable();
        sem.up_isr();
    };
    timer.set_callback(tos::function_ref<void()>(handler));
    timer.set_frequency(500);
    timer.enable();
    auto begin = tos::x86_64::rdtsc();
    sem.down();
    auto diff = end - begin;
    // diff has rdtsc ticks per 2ms
    return diff / 2'000;
}

void platform_support::stage2_init() {
    m_palloc = force_get(initialize_page_allocator());

    g_palloc = m_palloc;

    apic_initialize(*m_palloc);

    auto ticks_per_us = calibrate_tsc(m_chrono.timer);
    tos::debug::log("RDTSC ticks per us:", ticks_per_us);
    m_rdtsc_clock.m_impl.calibrate = ticks_per_us;

    tos::lwip::global::system_clock = &m_rdtsc_clock;

    init_pci(*m_palloc, get_registry());

    set_name(tos::launch(tos::alloc_stack,
                         [&] {
                             while (true) {
                                 using namespace std::chrono_literals;
                                 tos::this_thread::sleep_for(m_chrono.alarm, 50ms);
                                 tos::lock_guard lg{tos::lwip::lwip_lock};
                                 sys_check_timeouts();
                             }
                         }),
             "LWIP check timeouts");

        auto p = new tos::preempter<platform_support>{};
        p->setup(*this, 5);

        tos::ae::preemptive_user_group_runner::create(
            tos::mem_function_ref<&tos::preempter<platform_support>::run>(*p));

//    tos::ae::preemptive_user_group_runner::create(
//        tos::make_erased_preemptive_runner(*this));
}

platform_group_args platform_support::make_args() {
    return {.page_alloc = &*m_palloc,
            .trampoline = &*m_trampoline,
            .table = &tos::x86_64::get_current_translation_table()};
}

namespace tos::ae::x86_64 {}