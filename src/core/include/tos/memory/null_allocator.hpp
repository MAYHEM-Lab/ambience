#pragma once

#include <cstdlib>

namespace tos::memory {
class null_allocator {
public:
    void* allocate(size_t) {
        return nullptr;
    }
    void free(void*) {
    }
};
} // namespace tos::memory