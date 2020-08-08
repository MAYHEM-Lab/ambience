//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#include <new>
#include <tos/components/allocator.hpp>
#include <tos/context.hpp>
#include <tos/ft.hpp>

using tos::allocator_component;

namespace tos {
void out_of_memory_handler();

[[gnu::weak]]
void out_of_memory_handler() {
    LOG_WARN("Memory allocation failed!");
}
}

void operator delete(void* pt, size_t) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete(void* pt) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[](void* pt) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[](void* pt, size_t) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void* operator new(size_t sz) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler();
        }
        return res;
    }
    tos::out_of_memory_handler();
    return nullptr;
}

void* operator new[](size_t sz) {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler();
        }
        return res;
    }
    tos::out_of_memory_handler();
    return nullptr;
}

void* operator new(size_t sz, const std::nothrow_t&) noexcept {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler();
        }
        return res;
    }
    tos::out_of_memory_handler();
    return nullptr;
}

void* operator new[](size_t sz, const std::nothrow_t&) noexcept {
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        auto res = alloc->allocator->allocate(sz);
        if (res == nullptr) {
            tos::out_of_memory_handler();
        }
        return res;
    }
    tos::out_of_memory_handler();
    return nullptr;
}