#include <tos/ae/detail/syscall.hpp>
#include <tos/aarch64/assembly.hpp>

namespace tos::ae::detail {
namespace {
int64_t sysreq(int64_t num, uintptr_t data) {
    asm volatile(
    "mov x0, %[num]\n"
    "mov x1, %[data]\n"
    :
    :
    [num] "r"(num), [data] "r"(data)
    : "x0", "x1");
    tos::aarch64::svc2();
    int64_t result;
    asm volatile("mov %[result], x0" : [result] "=r"(result) : : "x0");
    return result;
}
}

void do_init_syscall(interface& iface) {
    sysreq(1, reinterpret_cast<uintptr_t>(&iface));
}

void do_yield_syscall() {
    sysreq(2, 0);
}
} // namespace tos::ae::detail