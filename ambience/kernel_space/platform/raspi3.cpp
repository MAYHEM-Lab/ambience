#include "kernel.hpp"
#include <arch/drivers.hpp>
#include <calc_generated.hpp>
#include <common/inet/tcp_ip.hpp>
#include <group1.hpp>
#include <nonstd/variant.hpp>
#include <tos/aarch64/exception.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/ae/transport/lwip/udp.hpp>
#include <tos/arch.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/elf.hpp>
#include <tos/flags.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/late_constructed.hpp>
#include <tos/periph/bcm2837_clock.hpp>
#include <tos/platform.hpp>

using errors = mpark::variant<tos::aarch64::mmu_errors>;
using tos::expected;

enum class usb_errors
{
    hci_init_fail,
    no_device
};

tos::expected<void, usb_errors> usb_task(tos::raspi3::interrupt_controller& ic,
                                         tos::any_clock& clk,
                                         tos::any_alarm& alarm);

class svc_on_demand_interrupt {
public:
    template<class T>
    void operator()(T&& t) {
        tos::cur_arch::exception::set_svc_handler(
            tos::cur_arch::exception::svc_handler_t(t));
        tos::cur_arch::svc1();
    }
};

class raspi3_platform_support {
public:
    void stage1_init() {
        tos::periph::clock_manager clock_man;
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
        LOG("Max CPU Freq:", clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        clock_man.set_frequency(tos::bcm283x::clocks::arm,
                                clock_man.get_max_frequency(tos::bcm283x::clocks::arm));
        LOG("CPU Freq:", clock_man.get_frequency(tos::bcm283x::clocks::arm));
    }

    void stage2_init() {
        tos::launch(tos::alloc_stack, [this] { usb_task(ic, m_clock, m_alarm); });
    }

    auto init_serial() {
        return tos::open(tos::devs::usart<0>, tos::uart::default_115200, ic);
    }

    auto init_timer() {
        return tos::raspi3::system_timer(ic);
    }

    auto init_odi() {
        return svc_on_demand_interrupt();
    }

    auto init_physical_memory_allocator() {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

        auto op_res = tos::aarch64::allocate_region(
            level0_table,
            tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
            tos::user_accessible::no,
            nullptr);
        if (!op_res) {
            LOG_ERROR("Could not allocate ...");
        }

        op_res = tos::aarch64::mark_resident(
            level0_table,
            tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
            tos::memory_types::normal,
            (void*)4096);
        if (!op_res) {
            LOG_ERROR("Could not mark resident ...");
        }

        tos::aarch64::tlb_invalidate_all();

        auto palloc =
            new (reinterpret_cast<void*>(4096)) tos::physical_page_allocator(1024);

        tos::aarch64::traverse_table_entries(
            level0_table, [&](tos::memory_range range, tos::aarch64::table_entry& entry) {
                LOG_TRACE("Making [",
                          (void*)range.base,
                          ",",
                          (void*)range.end(),
                          "] unavailable");

                entry.allow_user(true);
                palloc->mark_unavailable(range);
            });

        palloc->mark_unavailable({0, 4096});
        tos::aarch64::tlb_invalidate_all();

        return palloc;
    }

    struct sample_group_descr {
        static constexpr auto& elf_body = group1;
        static constexpr auto services = tos::meta::list<tos::ae::services::calculator>{};
    };

    std::vector<std::unique_ptr<tos::ae::kernel::user_group>>
    init_groups(tos::interrupt_trampoline& trampoline,
                tos::physical_page_allocator& palloc) {
        auto& level0_table = tos::cur_arch::get_current_translation_table();

        std::vector<std::unique_ptr<tos::ae::kernel::user_group>> runnable_groups;
        {
            runnable_groups.emplace_back(tos::ae::kernel::load_preemptive_elf_group(
                sample_group_descr{}, trampoline, palloc, level0_table));

            LOG("Group loaded");
        }

        return runnable_groups;
    }

    template<class FnT>
    void set_syscall_handler(FnT&& syshandler) {
        tos::aarch64::exception::set_svc_handler(
            tos::aarch64::exception::svc_handler_t(syshandler));
    }

    void return_to_thread_from_irq(tos::kern::tcb& from, tos::kern::tcb& to) {
        return_from = &from;
        return_to = &to;
        ic.set_post_irq(
            tos::mem_function_ref<&raspi3_platform_support::do_return>(*this));
    }

    using timer_mux_type = tos::timer_multiplexer<tos::raspi3::generic_timer, 3>;
    using channel_type = timer_mux_type::multiplexed_timer;
    using clock_type = tos::clock<channel_type>;
    using erased_clock_type = tos::detail::erased_clock<clock_type>;

    using alarm_type = tos::alarm<channel_type>;
    using erased_alarm_type = tos::detail::erased_alarm<alarm_type>;

private:
    void do_return() {
        ic.reset_post_irq();
        tos::swap_context(*return_from, *return_to, tos::int_ctx{});
    }

    tos::kern::tcb* return_from;
    tos::kern::tcb* return_to;

    tos::raspi3::interrupt_controller ic;

public:
    timer_mux_type m_tim_mux{ic, 0};
    erased_clock_type m_clock{m_tim_mux.channel(1)};
    erased_alarm_type m_alarm{m_tim_mux.channel(2)};
};

expected<void, errors> kernel() {
    tos::ae::manager<raspi3_platform_support> man;
    man.initialize();

    tos::debug::log_server serv(man.get_log_sink());

    man.groups().front()->exposed_services.emplace_back(tos::ae::service_host(&serv));

    auto& g = man.groups().front();

    auto req_task = [&g = man.groups().front()]() -> tos::Task<void> {
        auto serv = static_cast<tos::ae::services::calculator::async_server*>(
            g->channels.front().get());

        auto res = co_await serv->add(3, 4);
        tos::launch(tos::alloc_stack, [res] { LOG("3 + 4 =", res); });
    };

    tos::coro::make_detached(req_task);

    for (int i = 0; i < 10; ++i) {
        man.run();
    }

    tos::launch(tos::alloc_stack, [&man] {
        using namespace std::chrono_literals;
        tos::this_thread::sleep_for(man.m_alarm, 5s);

        tos::ae::services::calculator::stub_client<tos::ae::udp_transport> client{
            tos::udp_endpoint_t{tos::parse_ipv4_address("10.0.0.38"), {1993}}};
        LOG("In T1");

        for (int i = 0; i < 100; ++i) {
            for (int j = 0; j < 100; ++j) {
                LOG("[T1] [Host: 10.0.0.38:1993]", i, "+", j, "=", client.add(i, j));
            }
        }
    });

    tos::this_thread::block_forever();

    return {};
}

static tos::stack_storage kern_stack;
void tos_main() {
    tos::launch(kern_stack, kernel);
}
