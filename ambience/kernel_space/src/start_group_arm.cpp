#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/ae/kernel/start_group.hpp>

namespace tos::ae::kernel {
std::unique_ptr<user_group> start_group(tos::span<uint8_t> stack,
                                        void (*entry)(),
                                        tos::interrupt_trampoline& trampoline,
                                        std::string_view name) {
    auto& self = *tos::self();

    auto res = std::make_unique<user_group>();

    arm::mpu mpu;

    auto task_svc_handler = [&](int svc, arm::exception::stack_frame_t& frame) {
        arm::set_control(0);
        arm::isb();

        Assert(frame.r0 == 1);

        auto ifc = reinterpret_cast<interface*>(frame.r1);
        res->iface.user_iface = ifc;

        trampoline.switch_to(self);
    };

    auto pre_sched = [&] {
        // mpu.enable();
        mpu.set_region(0,
                       {reinterpret_cast<uintptr_t>(stack.data()),
                        static_cast<ptrdiff_t>(stack.size())},
                       permissions::read_write);
        mpu.set_region(1, {0x800'00'00, 1024 * 1024}, permissions::read_execute, false);
        arm::exception::set_svc_handler(arm::exception::svc_handler_t(task_svc_handler));

        arm::set_control(1);
        arm::isb();
    };

    res->state = &tos::suspended_launch(stack, [&] {
        pre_sched();
        entry();
    });
    set_name(*res->state, name);

    tos::swap_context(self, *res->state, tos::int_guard{});

    tos::debug::log("Group initialized, interface at", res->iface.user_iface);

    return res;
}
} // namespace tos::ae::kernel