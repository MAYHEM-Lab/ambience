#include <calc_generated.hpp>
#include <log_generated.hpp>
#include <tos/ae/group.hpp>
#include <tos/ae/transport/upcall.hpp>
#include <tos/ae/user_space.hpp>
#include <tos/allocator/free_list.hpp>
#include <tos/task.hpp>

[[gnu::section(".nozero")]] uint8_t heap[2048];
tos::memory::free_list alloc{heap};

[[gnu::noinline]] void* operator new(size_t sz) {
    auto ptr = alloc.allocate(sz);
    if (ptr == nullptr) {
        // TODO: handle this via a panic
    }
    return ptr;
}

[[gnu::noinline]] void operator delete(void* pt) {
    alloc.free(pt);
}

tos::ae::group<1>* g;

tos::Task<bool> handle_req(tos::ae::req_elem el) {
    co_return tos::ae::run_req(*g, el);
}

extern tos::ae::interface iface;

tos::Task<tos::ae::services::calculator::sync_server*> init_basic_calc();
tos::Task<void> task() {
    ::g = new tos::ae::group<1>(tos::ae::group<1>::make(co_await init_basic_calc()));

    tos::services::logger::async_zerocopy_client<tos::ae::upcall_transport> client(iface,
                                                                                   1);
    co_await client.start(tos::services::log_level::info);
    co_await client.log_string("Hello world!");
    co_await client.finish();

    while (true) {
        co_await tos::ae::log_str("Hello world from user space!");
        co_await tos::ae::log_str("Second hello world from user space!");
    }
}