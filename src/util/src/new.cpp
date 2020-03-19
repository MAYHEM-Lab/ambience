//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#include "../../core/include/tos/memory/free_list.hpp"

#include <new>

alignas(16) uint8_t heap[1024*1024];
tos::memory::free_list alloc(heap);

void operator delete (void* pt, size_t){
    alloc.free(pt);
}

void operator delete (void* pt)
{
    alloc.free(pt);
}

void operator delete[] (void* pt)
{
    alloc.free(pt);
}

void operator delete[] (void* pt, size_t)
{
    alloc.free(pt);
}

void* operator new(size_t sz)
{
    auto ptr = alloc.allocate(sz);
    return ptr;
}

void* operator new[](size_t sz)
{
    auto ptr = alloc.allocate(sz);
    return ptr;
}

void* operator new(size_t sz, const std::nothrow_t&) noexcept
{
    auto ptr = alloc.allocate(sz);
    return ptr;
}

void* operator new[](size_t sz, const std::nothrow_t&) noexcept
{
    auto ptr = alloc.allocate(sz);
    return ptr;
}
