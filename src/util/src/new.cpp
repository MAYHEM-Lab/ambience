//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#include <new>
#include <tos/context.hpp>
#include <tos/components/allocator.hpp>
#include <tos/ft.hpp>

using tos::allocator_component;

void operator delete (void* pt, size_t){
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete (void* pt)
{
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[] (void* pt)
{
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void operator delete[] (void* pt, size_t)
{
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        alloc->allocator->free(pt);
        return;
    }
}

void* operator new(size_t sz)
{
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        return alloc->allocator->allocate(sz);
    }
    return nullptr;
}

void* operator new[](size_t sz)
{
    if (auto alloc = tos::current_context().get_component<allocator_component>(); alloc) {
        return alloc->allocator->allocate(sz);
    }
    return nullptr;
}
