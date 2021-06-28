#pragma once

#include <common/clock.hpp>
#include <common/timer.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/late_constructed.hpp>
#include <tos/preemption.hpp>
#include <tos/ae/kernel/runners/preemptive_user_runner.hpp>

namespace tos::ae {
template<class T>
concept PlatformSupport = requires(T t) {
    t.stage1_init();
    t.init_serial();
    t.init_timer();
    t.init_odi();
    t.init_physical_memory_allocator();
    t.init_groups(std::declval<tos::interrupt_trampoline&>(),
                  *t.init_physical_memory_allocator());
};

template<PlatformSupport Platform>
class manager : public Platform {
    using serial_type = decltype(std::declval<Platform>().init_serial());
    using timer_type = decltype(std::declval<Platform>().init_timer());
    using odi_type = decltype(std::declval<Platform>().init_odi());

public:
    void initialize() {
        m_serial.emplace_fn(&Platform::init_serial, static_cast<Platform*>(this));
        tos::println(m_serial.get(), "ambience");

        m_sink.emplace(&m_serial.get());
        m_logger.emplace(&m_sink.get());
        tos::debug::set_default_log(&m_logger.get());
        LOG("Log setup complete");

        // Stage 1 initialization has a logger!
        Platform::stage1_init();

        m_timer.emplace_fn(&Platform::init_timer, static_cast<Platform*>(this));
        LOG("Timer setup complete");

        m_odi.emplace_fn(&Platform::init_odi, static_cast<Platform*>(this));
        m_trampoline = tos::make_interrupt_trampoline(m_odi.get());
        LOG("Trampoline setup complete");

        auto palloc = Platform::init_physical_memory_allocator();
        LOG("Page allocator intialized");

        Platform::stage2_init();

        m_runnable_groups = Platform::init_groups(*m_trampoline, *palloc);
        LOG("Groups initialized");
    }

    void run() {
        auto ctx = make_overload(
            [this](preempt_ops::get_timer_t) -> auto& { return m_timer.get(); },
            [this](preempt_ops::get_odi_t) -> auto& { return m_odi.get(); },
            [this](preempt_ops::set_syscall_handler_t, auto&&... args) {
                return Platform::set_syscall_handler(args...);
            },
            [this](preempt_ops::return_to_thread_from_irq_t, auto&&... args) {
                return Platform::return_to_thread_from_irq(args...);
            });

        preemptive_user_group_runner runner(make_erased_preemptive_runner(ctx));
        runner.run(m_runnable_groups.front());
        tos::this_thread::yield();
    }

    tos::span<const tos::ae::kernel::user_group> groups() const {
        return m_runnable_groups;
    }

    tos::span<tos::ae::kernel::user_group> groups() {
        return m_runnable_groups;
    }

    auto& get_log_sink() {
        return m_sink.get();
    }

private:
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline;
    std::vector<tos::ae::kernel::user_group> m_runnable_groups;

    tos::late_constructed<serial_type> m_serial;
    tos::late_constructed<tos::debug::serial_sink<serial_type*>> m_sink;
    tos::late_constructed<tos::debug::detail::any_logger> m_logger;
    tos::late_constructed<timer_type> m_timer;
    tos::late_constructed<odi_type> m_odi;
};
} // namespace tos::ae