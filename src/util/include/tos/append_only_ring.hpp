#pragma once

#include <concepts>
#include <iterator>
#include <tos/generator.hpp>
#include <tos/span.hpp>

namespace tos {
template<class SizeType>
struct append_only_ring {
    void* allocate(int size, int align = 1) {
        auto alloc_sz = alloc_len(size, align);

        // Backing store is smaller than the necessary space, cannot satisfy this
        // allocation.
        if (m_storage.size() < alloc_sz) {
            return nullptr;
        }

        if (std::distance(end, m_storage.end()) < alloc_sz) {
            int sum = 0;
            for (auto [ptr, sz] : allocations()) {
                sum += sz;
                if (sum >= alloc_sz) {
                    break;
                }
            }
            alloc_sz = sum;
            // wrap-around
            // Must invalidate as many allocations as needed until we make enough
            // space.
        }

        auto sz = new (end) SizeType(alloc_sz);
        end = end + sz;
        return sz + 1;
    }

    void clear() {
        head = nullptr;
        tail = nullptr;
        end = m_storage.begin();
    }

    Generator<std::pair<void*, SizeType>> allocations() {
        uint8_t* first = head;

        do {
            auto len = *reinterpret_cast<SizeType*>(first);
            co_yield std::pair<void*, int>(first, len);
            first += len;
        } while (first != tail);
    }
private:
    int alloc_len(int size, int align) {
        return sizeof(SizeType) + size;
    }

    // [size of allocation] [...data...] [size of next allocation] [...data...]
    span<uint8_t> m_storage;
    uint8_t* head = nullptr;
    uint8_t* tail = nullptr;
    uint8_t* end = m_storage.begin();
};
} // namespace tos