#include <tos/ae/detail/syscall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/mem_stream.hpp>
#include <tos/x86_64/assembly.hpp>

struct {
    char buf[1024];
    tos::omemory_stream str{buf};
} low_level_buffer;
namespace tos::ae {
NO_INLINE
int low_level_output_t::write(tos::span<const uint8_t> data) {
    low_level_buffer.str.write(data);
    if (low_level_buffer.str.get().size() == std::size(low_level_buffer.buf) ||
        low_level_buffer.str.get().back() == '\n' ||
        low_level_buffer.str.get().back() == '\r') {
        auto buffer = low_level_buffer.str.get();
        tos::x86_64::syscall(0xDEADBEEF, reinterpret_cast<uint64_t>(&buffer));
        low_level_buffer.str = tos::omemory_stream{low_level_buffer.buf};
    }
    return data.size();
}

uint64_t timestamp() {
    return tos::x86_64::rdtsc();
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