#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/interrupt_trampoline.hpp>
#include <tos/ae/kernel/start_group.hpp>

namespace tos::ae::kernel {
namespace {
void switch_to_el0(void (*el0_fn)(), void* stack, size_t stack_size) {
    LOG("Switching to EL0", (void*)el0_fn);
    tos::platform::disable_interrupts();
    uint64_t spsr_el1 = 0;
    tos::aarch64::set_spsr_el1(spsr_el1);
    tos::aarch64::set_elr_el1(reinterpret_cast<uintptr_t>(el0_fn));
    tos::aarch64::set_sp_el0(reinterpret_cast<uintptr_t>(stack) + stack_size);
    tos::aarch64::isb();
    tos::aarch64::eret();
}
} // namespace

std::unique_ptr<user_group>
start_group(span<uint8_t> stack, void (*entry)(), interrupt_trampoline& trampoline) {
    auto& self = *tos::self();

    auto res = std::make_unique<user_group>();

    auto svc_handler_ = [&](int svnum, cur_arch::exception::stack_frame_t& frame) {
        Assert(frame.gpr[0] == 1);

        auto ifc = reinterpret_cast<ae::interface*>(frame.gpr[1]);
        res->iface.user_iface = ifc;

        trampoline.switch_to(self);
    };
    aarch64::exception::set_svc_handler(aarch64::exception::svc_handler_t(svc_handler_));

    res->state = &suspended_launch(
        alloc_stack, [&] { switch_to_el0(entry, stack.data(), stack.size()); });

    swap_context(self, *res->state, int_guard{});

    return res;
}
} // namespace tos::ae::kernel
