//
// Created by Mehmet Fatih BAKIR on 14/05/2018.
//

#pragma once

#include <stddef.h>

inline void* operator new(size_t, void* __p) noexcept { return __p; }
inline void* operator new[](size_t, void* __p) noexcept { return __p; }

// Default placement versions of operator delete.
inline void  operator delete  (void*, void*) throw() { }
inline void  operator delete[](void*, void*) throw() { }
