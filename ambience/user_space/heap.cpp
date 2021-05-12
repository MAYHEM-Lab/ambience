#include <tos/allocator/free_list.hpp>

[[gnu::section(".nozero")]] uint8_t heap[2048];
tos::memory::free_list alloc{heap};

void* operator new(size_t sz) {
    auto ptr = alloc.allocate(sz);
    if (ptr == nullptr) {
        while (true);
        // TODO: handle this via a panic
    }
    return ptr;
}

void operator delete(void* pt) {
    alloc.free(pt);
}
