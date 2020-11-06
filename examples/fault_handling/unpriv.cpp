#include <tos/span.hpp>
#include <this_thread_generated.hpp>

extern "C" int sysreq(int channel, void* data, int size);

extern "C" [[gnu::naked]] int sysreq(int channel, void* data, int size) {
    asm volatile("svc #0x6F \n"
                 "bx lr"
    :
    :
    : "memory");
}

int sysreq(int channel, tos::span<uint8_t> data) {
    return ::sysreq(channel, data.data(), data.size());
}

struct syscall_transport {
public:
    tos::span<uint8_t> get_buffer() {
        return m_buf;
    }

    tos::span<uint8_t> send_receive(tos::span<uint8_t> buf) {
        auto res = sysreq(1, buf);
        if (res > 0) {
            return buf.slice(res);
        }
        return tos::span<uint8_t> {nullptr};
    }

private:

    alignas(32) uint8_t m_buf[128];
};

using sc_thread_man = tos::services::remote_threadman<syscall_transport>;

void fault_main() {
    uint32_t thread_id = 42;
    sc_thread_man tman;
    tman.schedule(thread_id); // i.e. yield
    tman.kill(thread_id); // i.e. exit
    asm volatile ("udf 255");
}
