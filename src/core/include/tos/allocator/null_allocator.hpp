#pragma once

#include <cstdlib>
#include <optional>

namespace tos::memory {
class null_allocator {
public:
    void* allocate(size_t) {
        return nullptr;
    }
    void free(void*) {
    }
    void* realloc(void*, size_t) {
        return nullptr;
    }
    std::optional<size_t> in_use() const {
        return {};
    }
    std::optional<size_t> peak_use() const {
        return {};
    }
    std::optional<size_t> capacity() const {
        return {};
    }
};
} // namespace tos::memory