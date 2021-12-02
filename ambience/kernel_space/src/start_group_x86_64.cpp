#include <tos/ae/kernel/start_group.hpp>
#include <tos/ae/kernel/user_group.hpp>
#include <tos/arch.hpp>
#include <tos/interrupt_trampoline.hpp>

namespace tos::ae::kernel {
namespace {
void switch_to_user(void* user_code) {
    using namespace tos::x86_64;
    asm volatile("add $8, %rsp");
    asm volatile("movq $0, %rbp");
    asm volatile("movq $0x202, %r11");
    sysret(user_code);
}
} // namespace

std::unique_ptr<user_group>
start_group(span<uint8_t> stack, void (*entry)(), interrupt_trampoline& trampoline,
            std::string_view name,
            tos::cur_arch::address_space& as) {
    LOG(name, "Entry point:", (void*)entry);
    auto& user_thread = tos::suspended_launch(stack, switch_to_user, (void*)entry);
    set_name(user_thread, name);

    auto& self = *tos::self();

    auto res = std::make_unique<user_group>();

    if (!res) {
        tos::debug::error("Could not allocate user group!");
        return nullptr;
    }

    res->state = &user_thread;

    auto syshandler = [&](tos::cur_arch::syscall_frame& frame) {
        assert(frame.rdi == 1);

        auto ifc = reinterpret_cast<tos::ae::interface*>(frame.rsi);
        res->iface.user_iface = ifc;

        trampoline.switch_to(self);
    };

    x86_64::set_syscall_handler(cur_arch::syscall_handler_t(syshandler));

    auto cur_as = tos::global::cur_as;
    activate(as);
    tos::swap_context(self, user_thread, int_guard{});
    activate(*cur_as);

    tos::debug::log("Group initialized, interface at", res->iface.user_iface);

    return res;
}
} // namespace tos::ae::kernel