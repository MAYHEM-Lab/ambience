#include <calc_generated.hpp>
#include <nonstd/variant.hpp>
#include <tos/address_space.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arm/exception.hpp>
#include <tos/board.hpp>
#include <tos/debug/dynamic_log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
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

alignas(4) uint8_t stk[1024];
expected<tos::ae::kernel::user_group, errors> start_group(
    tos::span<uint8_t> stack, void (*entry)(), tos::interrupt_trampoline& trampoline) {
    auto& self = *tos::self();

    tos::ae::kernel::user_group res;

    tos::arm::mpu mpu;

    auto task_svc_handler = [&](int svc, tos::arm::exception::stack_frame_t& frame) {
        tos::arm::set_control(0);
        tos::arm::isb();

        Assert(frame.r0 == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.r1);
        res.iface.user_iface = ifc;

        trampoline.switch_to(self);
    };

    auto pre_sched = [&] {
        // mpu.enable();
        mpu.set_region(0,
                       {reinterpret_cast<uintptr_t>(stack.data()),
                        static_cast<ptrdiff_t>(stack.size())},
                       tos::permissions::read_write);
        mpu.set_region(
            1, {0x800'00'00, 1024 * 1024}, tos::permissions::read_execute, false);
        tos::arm::exception::set_svc_handler(
            tos::arm::exception::svc_handler_t(task_svc_handler));

        //        LOG("MPU setup complete, allowing stack & flash access");

        //        LOG("Switching to unprivileged mode");
        tos::arm::set_control(1);
        tos::arm::isb();
    };

    res.state = &tos::suspended_launch(stack, [&] {
        pre_sched();
        entry();
    });

    tos::swap_context(self, *res.state, tos::int_guard{});

    return res;
}

expected<void, errors> kernel() {
    auto log_backend = bs::default_com::open();
    tos::println(log_backend, "ambience");

    tos::debug::serial_sink sink(&log_backend);
    tos::debug::detail::any_logger logger(&sink);
    tos::debug::set_default_log(&logger);

    LOG("Log setup complete");

    auto timer = tos::open(tos::devs::timer<2>);
    LOG("Timer setup complete");

    svc_on_demand_interrupt odi{};
    auto trampoline = tos::make_interrupt_trampoline(odi);

    LOG("Trampoline setup complete");

    std::vector<tos::ae::kernel::user_group> runnable_groups;

    runnable_groups.push_back(force_get(
        start_group(stk, reinterpret_cast<void (*)()>(0x8020d79), *trampoline)));

    LOG("Group started");
    int32_t x = 3, y = 42;
    auto params = std::make_tuple(&x, &y);
    auto results = tos::ae::service::calculator::wire_types::add_results{-1};

    auto& req1 = tos::ae::submit_req<true>(
        *runnable_groups.front().iface.user_iface, 0, 0, &params, &results);

    auto& self = *tos::self();

    auto syshandler2 = [&](int svc, tos::arm::exception::stack_frame_t& frame) {
        tos::arm::set_control(0);
        tos::arm::isb();

        tos::swap_context(*runnable_groups.front().state, self, tos::int_ctx{});
    };

    bool preempted = false;
    auto preempt = [&] {
        tos::arm::set_control(0);
        tos::arm::isb();

        preempted = true;
        tos::swap_context(*runnable_groups.front().state, self, tos::int_ctx{});
    };
    timer.set_callback(tos::function_ref<void()>(preempt));
    timer.set_frequency(10);

    for (int i = 0; i < 10; ++i) {
        LOG("back", results.ret0(), "Preempted", preempted);
        proc_req_queue(runnable_groups.front().iface);
        tos::this_thread::yield();

        odi([&](auto...) {
            tos::arm::exception::set_svc_handler(
                tos::arm::exception::svc_handler_t(syshandler2));

            preempted = false;
            timer.enable();
            tos::swap_context(self, *runnable_groups.front().state, tos::int_ctx{});
            timer.disable();
        });
        // std::swap(runnable_groups.front(), runnable_groups.back());
    }

    return {};
}

static tos::stack_storage kern_stack;
void tos_main() {
    tos::launch(kern_stack, kernel);
}