#include <new>
#include <tos/ae/user_space.hpp>
#include <tos/allocator/free_list.hpp>
#include <tos/compiler.hpp>
#include <tos/debug/debug.hpp>
#include <tos/debug/panic.hpp>

NO_ZERO uint8_t heap[4096*7500];

auto& alloc() {
    static tos::memory::free_list alloc{heap};
    return alloc;
}

tos::memory::polymorphic_allocator& tos::ae::default_allocator() {
    static auto erased = tos::memory::erase_allocator(&alloc());
    return erased;
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

void* operator new[](size_t sz, const std::nothrow_t&) noexcept {
    return operator new(sz);
}

void operator delete[](void* ptr) {
    operator delete(ptr);
}