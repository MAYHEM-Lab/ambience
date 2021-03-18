#include <tos/task.hpp>
#include <tos/memory/free_list.hpp>
#include <tos/ae/user_space.hpp>

[[gnu::section(".nozero")]]
uint8_t heap[512];
tos::memory::free_list alloc{heap};

[[gnu::noinline]]
void* operator new(size_t sz) {
    return alloc.allocate(sz);
}

[[gnu::noinline]]
void operator delete(void* pt) {
    alloc.free(pt);
}

tos::Task<void> task() {
    while (true) {
        co_await tos::ae::log_str("Hello world from user space!");
        co_await tos::ae::log_str("Second hello world from user space!");
    }
}