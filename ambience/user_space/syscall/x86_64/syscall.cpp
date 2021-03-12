#include <tos/ae/detail/syscall.hpp>
#include <tos/x86_64/assembly.hpp>

namespace tos::ae::detail {
void do_init_syscall(interface& iface) {
    tos::x86_64::syscall(1, reinterpret_cast<uint64_t>(&iface));
}

void do_yield_syscall() {
    tos::x86_64::syscall(2, 0);
}
} // namespace tos::ae::detail