#include <new>
#include <tos/allocator/free_list.hpp>
#include <tos/debug/debug.hpp>
#include <tos/debug/panic.hpp>
#include <tos/compiler.hpp>

NO_ZERO uint8_t heap[4096*7500];

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

void* operator new[](size_t sz) {
    return operator new(sz);
}

void* operator new[](size_t sz, const std::nothrow_t&) {
    return operator new(sz);
}

void operator delete[](void* ptr) {
    operator delete(ptr);
}