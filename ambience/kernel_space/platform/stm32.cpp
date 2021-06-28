#include "kernel.hpp"
#include <alarm_generated.hpp>
#include <block_memory_generated.hpp>
#include <calc_generated.hpp>
#include <nonstd/variant.hpp>
#include <tos/address_space.hpp>
#include <tos/ae/kernel/loaders/in_memory_user.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/ae/transport/downcall.hpp>
#include <tos/arm/exception.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/lidl_sink.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/tos_bind.hpp>
#include <tos/expected.hpp>
#include <tos/ft.hpp>
#include <tos/interrupt_trampoline.hpp>

using bs = tos::bsp::board_spec;
using errors = mpark::variant<tos::arm::mpu_errors>;
using tos::expected;

class svc_on_demand_interrupt {
public:
    svc_on_demand_interrupt() {
        NVIC_EnableIRQ(SVCall_IRQn);
        NVIC_SetPriority(SVCall_IRQn, 0);
    }

    template<class T>
    void operator()(T&& t) {
        tos::arm::exception::set_svc_handler(tos::arm::exception::svc_handler_t(t));
        tos::arm::svc1();
    }
};

alignas(4) uint8_t stk[2048];
class stm32_platform_support {
public:
    void stage1_init() {
    }
    void stage2_init() {
    }
    auto init_serial() {
        return bs::default_com::open();
    }

    auto init_timer() {
        return tos::open(tos::devs::timer<2>);
    }

    auto init_odi() {
        return svc_on_demand_interrupt();
    }

    tos::physical_page_allocator* init_physical_memory_allocator() {
        return nullptr;
    }

    std::vector<std::unique_ptr<tos::ae::kernel::user_group>>
    init_groups(tos::interrupt_trampoline& trampoline,
                tos::physical_page_allocator& palloc) {
        std::vector<std::unique_ptr<tos::ae::kernel::user_group>> runnable_groups;

        runnable_groups.push_back(tos::ae::kernel::do_load_preemptive_in_memory_group(
            reinterpret_cast<void (*)()>(0x8021129), trampoline));

        runnable_groups.back()->channels.push_back(
            std::make_unique<tos::ae::services::calculator::async_zerocopy_client<
                tos::ae::downcall_transport>>(*runnable_groups.back()->iface.user_iface,
                                              0));

        return runnable_groups;
    }

    template<class FnT>
    void set_syscall_handler(FnT&& syshandler) {
        tos::arm::exception::set_svc_handler(
            tos::arm::exception::svc_handler_t(syshandler));
    }

    void return_to_thread_from_irq(tos::kern::tcb& from, tos::kern::tcb& to) {
        tos::swap_context(from, to, tos::int_ctx{});
    }
};

template<class BaseAlarm>
struct async_alarm_impl : tos::ae::services::alarm::async_server {
    template<class... ArgTs>
    async_alarm_impl(ArgTs&&... args)
        : alarm(std::forward<ArgTs>(args)...) {
    }

    tos::Task<bool> sleep_for(tos::ae::services::milliseconds dur) override {
        co_await tos::async_sleep_for(alarm, std::chrono::milliseconds(dur.count()));
        co_return true;
    }

    BaseAlarm alarm;
};

tos::ae::services::block_memory::sync_server* init_stm32_flash_serv();
expected<void, errors> kernel() {
    tos::ae::manager<stm32_platform_support> man;
    man.initialize();

    auto blk_mem = init_stm32_flash_serv();

    auto buf = std::vector<uint8_t>(512);
    lidl::message_builder builder{buf};
    auto res = blk_mem->read(0, 0, 512, builder);
    LOG(res);

    auto tim = tos::open(tos::devs::timer<3>);
    auto alarm = tos::alarm(&tim);

    async_alarm_impl<decltype(&alarm)> async_alarm{&alarm};

    tos::debug::log_server serv(man.get_log_sink());

    man.groups().front()->exposed_services.emplace_back(tos::ae::sync_service_host(&serv));
    man.groups().front()->exposed_services.emplace_back(
        tos::ae::async_service_host(&async_alarm));

    auto& g = man.groups().front();

    auto req_task = [&g = man.groups().front(), &async_alarm]() -> tos::Task<void> {
        auto serv = static_cast<tos::ae::services::calculator::async_server*>(
            g->channels.front().get());

        auto res = co_await serv->add(3, 4);

        tos::launch(tos::alloc_stack, [res] { LOG("3 + 4 =", res); });
    };

    tos::coro::make_detached(req_task());

    for (int i = 0; i < 1000000; ++i) {
        man.run();
    }

    tos::this_thread::block_forever();
}

static tos::stack_storage kern_stack;
void tos_main() {
    tos::launch(kern_stack, kernel);
}