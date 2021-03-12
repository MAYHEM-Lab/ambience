#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>
#include <tos/ae/detail/syscall.hpp>
#include <memory>

tos::Task<void> task();

tos::ae::ring_elem elems[4];
tos::ae::interface iface{4, elems};
void user_code() {
    iface.req = new (new uint8_t[sizeof(tos::ae::ring) + iface.size * sizeof(uint16_t)])
        tos::ae::ring{};
    iface.res = new (new uint8_t[sizeof(tos::ae::ring) + iface.size * sizeof(uint16_t)])
        tos::ae::ring{};
    tos::ae::detail::do_init_syscall(iface);

    auto pollable = new tos::coro::pollable(tos::coro::make_pollable(task()));
    pollable->run();

    while (true) {
        tos::ae::detail::do_yield_syscall();
        proc_res_queue(iface);
    }
}
