#pragma once

#include <alarm_generated.hpp>
#include <common/alarm.hpp>
#include <nonstd/variant.hpp>
#include <tos/ae/registry.hpp>
#include <tos/arch.hpp>
#include <tos/expected.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/platform.hpp>
#include <tos/x86_64/pit.hpp>

using page_alloc_res = mpark::variant<tos::cur_arch::mmu_errors>;

tos::expected<tos::physical_page_allocator*, page_alloc_res> initialize_page_allocator();

struct timer
    : tos::x86_64::pit
    , tos::self_pointing<timer> {
    timer() {
        ensure(tos::platform::take_irq(0));
        tos::platform::set_irq(0, tos::mem_function_ref<&timer::irq>(*this));
    }

    timer(const timer&) = delete;
    timer(timer&&) = delete;

    void enable() {
        //            tos::x86_64::pic::enable_irq(0);
    }

    void disable() {
        //            tos::x86_64::pic::disable_irq(0);
    }

    void set_callback(tos::function_ref<void()> cb) {
        m_cb = cb;
    }

    void irq(tos::x86_64::exception_frame* frame, int) {
        m_cb();
    }

    tos::function_ref<void()> m_cb{[](void*) {}};
};

struct on_demand_interrupt {
    int irq;
    on_demand_interrupt() {
        irq = 12;
        ensure(tos::platform::take_irq(irq));
    }
    template<class T>
    void operator()(T&& t) {
        tos::platform::set_irq(irq, tos::platform::irq_handler_t(t));
        tos::cur_arch::int0x2c();
    }
};

void do_acpi_stuff();
void init_pci(tos::physical_page_allocator& palloc, tos::ae::registry_base& registry);

template<class BaseAlarm>
struct basic_async_alarm_impl : tos::ae::services::alarm::async_server {
    template<class... ArgTs>
    explicit basic_async_alarm_impl(ArgTs&&... args)
        : alarm(std::forward<ArgTs>(args)...) {
    }

    tos::Task<bool> sleep_for(tos::ae::services::milliseconds dur) override {
        co_await tos::async_sleep_for(alarm, std::chrono::milliseconds(dur.count()));
        co_return true;
    }

    BaseAlarm alarm;
};

using async_any_alarm_impl = basic_async_alarm_impl<tos::any_alarm*>;
