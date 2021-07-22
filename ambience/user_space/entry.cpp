#include <list>
#include <memory>
#include <tos/ae/detail/syscall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/debug/detail/logger_base.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
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

tos::debug::detail::any_logger* g_logger;

namespace tos::debug {
detail::any_logger& default_log() {
    return *g_logger;
}
}

#if defined(__x86_64__)
[[gnu::force_align_arg_pointer]]
#endif
[[gnu::used, noreturn]] extern "C" void _user_code() {
//    auto data = tos::default_segments::data();
//    auto data_start = reinterpret_cast<uint64_t*>(data.base);
//
//    // Copy initialized data
//    std::copy_n(&_sidata, data.size, data_start);

    auto bss = tos::default_segments::bss();
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base);
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end());

    std::uninitialized_value_construct_n(iface.elems, iface.size);

    // Zero out BSS
    std::fill(bss_start, bss_end, 0);

    // Call constructors
    std::for_each(start_ctors, end_ctors, [](auto x) { x(); });

    tos::debug::serial_sink sink(&tos::ae::low_level_output);
    tos::debug::detail::any_logger logger(&sink);
    logger.set_log_level(tos::debug::log_level::log);
    g_logger = &logger;
    tos::debug::log("Call init syscall");

    tos::ae::detail::do_init_syscall(iface);

    auto pollable = tos::coro::make_pollable(task());
    pollable.run();

    while (true) {
        proc_res_queue(iface);
        tos::ae::detail::do_yield_syscall();
    }
}
