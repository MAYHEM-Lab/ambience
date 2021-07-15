#include <tos/allocator/free_list.hpp>
#include <tos/debug/debug.hpp>

[[gnu::section(".nozero")]] uint8_t heap[4096*100];

auto& alloc() {
    static tos::memory::free_list alloc{heap};
    return alloc;
}

void* operator new(size_t sz) {
    auto ptr = alloc().allocate(sz);
    if (ptr == nullptr) {
        tos::debug::do_not_optimize(&sz);
        while (true);
        // TODO: handle this via a panic
    }
    return ptr;
}

void operator delete(void* pt) {
    alloc().free(pt);
}
