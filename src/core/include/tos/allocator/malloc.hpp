#pragma once

#include <cstdlib>
#include <optional>
#include <tos/debug/log.hpp>

namespace tos::memory {
class mallocator {
public:
    void* allocate(size_t size) {
        return ::malloc(size);
    }
    void free(void* ptr) {
        ::free(ptr);
    }
    void* realloc(void* oldptr, size_t newsz) {
        return ::realloc(oldptr, newsz);
    }
    std::optional<size_t> in_use() const {
        return {};
    }
};
}