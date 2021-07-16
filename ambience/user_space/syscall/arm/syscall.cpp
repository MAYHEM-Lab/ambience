#include <tos/ae/detail/syscall.hpp>
#include <tos/arm/assembly.hpp>

namespace tos::ae {
int low_level_output_t::write(tos::span<const uint8_t> data) {
    return data.size();
}
} // namespace tos::ae

namespace tos::ae::detail {
namespace {
int sysreq(int num, uintptr_t data);

[[gnu::naked]] int sysreq(int num, uintptr_t data) {
    asm volatile("svc #0x6F \n"
                 "bx lr"
                 :
                 :
                 : "memory");
}
} // namespace

void do_init_syscall(interface& iface) {
    sysreq(1, reinterpret_cast<uintptr_t>(&iface));
}

void do_yield_syscall() {
    sysreq(2, 0);
}
} // namespace tos::ae::detail