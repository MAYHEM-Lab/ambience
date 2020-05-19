#pragma once

#include <cstdlib>

namespace tos::memory {
class mallocator {
public:
    void* allocate(size_t size) {
        return ::malloc(size);
    }
    void free(void* ptr) {
        ::free(ptr);
    }
};
}