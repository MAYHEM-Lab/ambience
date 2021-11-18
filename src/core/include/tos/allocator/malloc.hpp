#pragma once

#include <cstdlib>
#include <optional>

namespace tos::memory {
class mallocator {
public:
    void* allocate(size_t size) {
        return ::malloc(size);
    }
    void free(void* ptr) {
        ::free(ptr);
    }
    std::optional<size_t> in_use() const {
        return {};
    }
};
}