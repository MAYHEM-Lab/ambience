#include <group_generated.hpp>
#include <list>
#include <memory>
#include <tos/ae/detail/syscall.hpp>
#include <tos/ae/group.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/debug/detail/logger_base.hpp>
#include <tos/debug/log.hpp>
#include <tos/debug/sinks/serial_sink.hpp>
#include <tos/detail/poll.hpp>
#include <tos/memory.hpp>

extern tos::ae::interface iface;

namespace {
struct user_group_impl : tos::ae::user_group::async_server {
    tos::Task<uint64_t> current_heap_use() override {
        co_return tos::ae::default_allocator().in_use().value();
    }
    tos::Task<uint64_t> peak_heap_use() override {
        co_return tos::ae::default_allocator().peak_use().value();
    }
    tos::Task<uint64_t> heap_capacity() override {
        co_return tos::ae::default_allocator().capacity().value();
    }
    tos::Task<uint64_t> current_concurrency() override {
        co_return tos::ae::concurrency.get()[1];
    }
    tos::Task<uint64_t> max_concurrency() override {
        co_return tos::ae::concurrency.get()[2];
    }
    tos::Task<uint64_t> iface_capacity() override {
        co_return iface.size;
    }
    tos::Task<uint64_t> queue_depth() override {
        co_return iface.host_to_guest_queue_depth();
    }
};
} // namespace

tos::Task<tos::ae::user_group::async_server*> init_user_group() {
    co_return new user_group_impl;
}

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
} // namespace tos::debug

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
    auto bss_start = reinterpret_cast<uint64_t*>(bss.base.direct_mapped());
    auto bss_end = reinterpret_cast<uint64_t*>(bss.end().direct_mapped());

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
