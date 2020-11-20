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
    std::optional<size_t> in_use() const {
        return {};
    }
};
} // namespace tos::memory