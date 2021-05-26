#include <list>
#include <memory>
#include <tos/ae/detail/syscall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/detail/poll.hpp>
#include <tos/memory.hpp>

extern tos::ae::interface iface;

tos::Task<void> task();

extern "C" {
extern void (*start_ctors[])(void);
extern void (*end_ctors[])(void);

extern uint64_t _sidata;

void abort() {
    while (true)
        ;
}

void __cxa_atexit() {
}
}

[[gnu::used, noreturn]] extern "C" void _user_code() {
//    auto data = tos::default_segments::data();
//    auto data_start = reinterpret_cast<uint64_t*>(data.base);
//
//    // Copy initialized data
//    std::copy_n(&_sidata, data.size, data_start);

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base);
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end());

    // Zero out BSS
    std::fill(bss_start, bss_end, 0);

    // Call constructors
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });

    tos::ae::detail::do_init_syscall(iface);

    auto pollable = tos::coro::make_pollable(task());
    pollable.run();

    while (true) {
        tos::ae::detail::do_yield_syscall();
        proc_res_queue(iface);
    }
}
