#include <tos/allocator/free_list.hpp>
#include <tos/debug/debug.hpp>
#include <tos/debug/panic.hpp>

[[gnu::section(".nozero")]] uint8_t heap[4096*7500];

auto& alloc() {
    static tos::memory::free_list alloc{heap};
    return alloc;
}

void* operator new(size_t sz) {
    auto ptr = alloc().allocate(sz);
    if (ptr == nullptr) {
        tos::debug::do_not_optimize(&sz);
        tos::debug::panic("Allocation failure");
    }
    return ptr;
}

void operator delete(void* pt) {
    alloc().free(pt);
}
