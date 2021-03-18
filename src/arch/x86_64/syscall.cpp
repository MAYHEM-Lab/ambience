#include <tos/x86_64/msr.hpp>
#include <tos/x86_64/syscall.hpp>

extern "C" {
void raw_syscall_entry();
}

namespace tos::x86_64 {
namespace {
syscall_handler_t syscall_handler([](auto&, void*) {});
}

syscall_handler_t set_syscall_handler(syscall_handler_t handler) {
    std::swap(handler, syscall_handler);
    return handler;
}

void initialize_syscall_support() {
    wrmsr(msrs::ia32_efer, rdmsr(msrs::ia32_efer) | 1ULL);

    wrmsr(msrs::star, (uint64_t(0x18ULL | 0x3ULL) << 48U) | (uint64_t(0x8ULL) << 32U));
    wrmsr(msrs::lstar, reinterpret_cast<uint64_t>(&raw_syscall_entry));
    wrmsr(msrs::sfmask, 0);
}
} // namespace tos::x86_64

extern "C" void syscall_entry(tos::x86_64::syscall_frame* frame) {
    frame->cs = 0x28 | 0x3;
    frame->ss = 0x20 | 0x3;
    frame->rsp += 8;
    tos::x86_64::syscall_handler(*frame);
}
