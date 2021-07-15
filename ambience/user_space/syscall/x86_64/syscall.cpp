#include <tos/ae/detail/syscall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/mem_stream.hpp>

namespace tos::ae {
int low_level_output_t::write(tos::span<const uint8_t> data) {
    static struct {
        char buf[1024];
        tos::omemory_stream str{buf};
    } x;
    x.str.write(data);
    if (x.str.get().size() == std::size(x.buf) || x.str.get().back() == '\n' ||
        x.str.get().back() == '\r') {
        auto buffer = x.str.get();
        tos::x86_64::syscall(0xDEADBEEF, reinterpret_cast<uint64_t>(&buffer));
        x.str = tos::omemory_stream{x.buf};
    }
    return data.size();
}
} // namespace tos::ae

namespace tos::ae::detail {
void do_init_syscall(interface& iface) {
    tos::x86_64::syscall(1, reinterpret_cast<uint64_t>(&iface));
}

void do_yield_syscall() {
    tos::x86_64::syscall(2, 0);
}
} // namespace tos::ae::detail