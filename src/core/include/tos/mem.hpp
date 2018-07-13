//
// Created by fatih on 7/12/18.
//

#pragma once

#include <stddef.h>
#include <tos/utility.hpp>

namespace tos
{
    /**
     * This class wraps the architecture dependent memory allocation routines.
     * This will usually use a malloc/free implementation if a libc is available.
     * If a libc is unavailable, the architecture should provide it's own allocator.
     */
    class arch_allocator
    {
        /**
         * Allocates enough memory to store at least the amount requested.
         *
         * The returned memory is not guaranteed to be aligned properly.
         *
         * Returns `nullptr` if the request cannot be satisfied.
         *
         * @param bytes bytes at least to allocate
         * @return pointer to the beginning of the memory
         */
        void* alloc(size_t bytes);

        /**
         * Returns a previously allocated block of memory to the system.
         *
         * If `nullptr` is passed, the call is a no-op.
         *
         * The given pointer must point to the beginning of a previously allocated
         * block of memory, otherwise the behaviour is undefined.
         *
         * @param mem pointer to the beginning of a previously allocated block of memory
         */
        void free(void* mem);
    };

    template <class BaseAllocatorT>
    class limited_allocator
    {
    public:
        limited_allocator(BaseAllocatorT& alloc, size_t max)
                : m_allocator{&alloc}, m_max{max}, m_used{} {};

        void* alloc(size_t bytes)
        {
            return m_allocator->alloc(bytes);
        }

        void free(void* mem)
        {
            m_allocator->free(mem);
        }

    private:
        BaseAllocatorT* m_allocator;
        size_t m_max;
        size_t m_used;
    };
}