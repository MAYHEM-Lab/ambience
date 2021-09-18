#include <tos/ae/detail/syscall.hpp>
#include <tos/arm/core.hpp>
#include <tos/mem_stream.hpp>

namespace {
int sysreq(unsigned int num, uintptr_t data);

[[gnu::naked]] int sysreq(unsigned int num, uintptr_t data) {
    asm volatile("svc #0x3 \n"
                 "bx lr"
                 :
                 :
                 : "memory");
}
} // namespace

namespace tos::ae {
int low_level_output_t::write(tos::span<const uint8_t> data) {
    static struct {
        char buf[256];
        tos::omemory_stream str{buf};
    } x;
    x.str.write(data);
    if (x.str.get().size() == std::size(x.buf) || x.str.get().back() == '\n' ||
        x.str.get().back() == '\r') {
        auto buffer = x.str.get();
        sysreq(0xDEADBEEF, reinterpret_cast<uint64_t>(&buffer));
        x.str = tos::omemory_stream{x.buf};
    }
    return data.size();
}

uint64_t timestamp() {
    return tos::arm::DWT::CYCCNT.read();
}
} // namespace tos::ae

namespace tos::ae::detail {
void do_init_syscall(interface& iface) {
    sysreq(1, reinterpret_cast<uintptr_t>(&iface));
}

void do_yield_syscall() {
    sysreq(2, 0);
}
} // namespace tos::ae::detail