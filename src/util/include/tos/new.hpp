//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#pragma once
#include <stddef.h>
#include <stdlib.h>

#if !defined(TOS_STDLIB_COMPAT)
#include <new>
#else


inline void* operator new(size_t, void* __p) noexcept { return __p; }
inline void* operator new[](size_t, void* __p) noexcept { return __p; }

// Default placement versions of operator delete.
inline void  operator delete  (void*, void*) throw() { }
inline void  operator delete[](void*, void*) throw() { }

#endif

inline void operator delete (void*, size_t){}
inline void operator delete (void* pt)
{
    free(pt);
}

inline void* operator new(size_t sz)
{
    return malloc(sz);
}