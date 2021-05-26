#pragma once

#include <common/clock.hpp>
#include <common/timer.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/late_constructed.hpp>

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

        m_self = tos::self();
    }

    bool run() {
        auto& tmr = m_timer.get();
        auto syshandler = [this, &tmr](auto&&...) {
            tmr.disable();
            tos::swap_context(*m_runnable_groups.front().state, *m_self, tos::int_ctx{});
        };

        bool preempted = false;
        auto preempt = [&] {
            tmr.disable();

            preempted = true;
            Platform::return_to_thread_from_irq(*m_runnable_groups.front().state,
                                                *m_self);
        };

        tmr.set_callback(tos::function_ref<void()>(preempt));
        tmr.set_frequency(200);

        m_odi.get()([&](auto...) {
            Platform::set_syscall_handler(syshandler);

            tmr.enable();

            //            tos::arm::set_control(1);
            //            tos::arm::isb();

            tos::swap_context(*m_self, *m_runnable_groups.front().state, tos::int_ctx{});

            //            tos::arm::set_control(0);
            //            tos::arm::isb();
        });

        proc_req_queue(
            [&](tos::ae::req_elem& req, auto done) {
                if (req.channel == 0) {
                    if (req.ret_ptr) {
                        *((volatile bool*)req.ret_ptr) = true;
                    }

                    if (req.procid == 1) {
                        LOG(*(std::string_view*)req.arg_ptr);
                    }

                    return done();
                }

                if (m_runnable_groups.front().exposed_services.size() <= req.channel - 1) {
                    LOG_ERROR("No such service!");
                    return done();
                }

                auto& channel =
                    m_runnable_groups.front().exposed_services[req.channel - 1];

                if (auto sync = get_if<tos::ae::sync_service_host>(&channel)) {
                    tos::launch(tos::alloc_stack, [sync, done, req] {
                        sync->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr);
                        return done();
                    });
                    return;
                }

                auto async = get_if<tos::ae::async_service_host>(&channel);

                tos::coro::make_detached(
                    async->run_zerocopy(req.procid, req.arg_ptr, req.ret_ptr), done);
            },
            m_runnable_groups.front().iface);

        tos::this_thread::yield();

        return preempted;
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
    tos::kern::tcb* m_self;
    std::unique_ptr<tos::interrupt_trampoline> m_trampoline;
    std::vector<tos::ae::kernel::user_group> m_runnable_groups;

    tos::late_constructed<serial_type> m_serial;
    tos::late_constructed<tos::debug::serial_sink<serial_type*>> m_sink;
    tos::late_constructed<tos::debug::detail::any_logger> m_logger;
    tos::late_constructed<timer_type> m_timer;
    tos::late_constructed<odi_type> m_odi;
};
} // namespace tos::ae