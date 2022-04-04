//
// Created by Fatih on 2/17/2020.
//

#include <algorithm>
#include <tos/allocator/free_list.hpp>
#include <tos/debug/log.hpp>
#include <tos/intrusive_list.hpp>

namespace tos::memory {
struct free_header : list_node<free_header> {
    explicit free_header(size_t sz)
        : size(sz) {
    }
    size_t size;
};

#define TOS_FREE_LIST_DEBUG 1
#undef TOS_FREE_LIST_DEBUG

#define TOS_FREE_LIST_CANARY 1
#undef TOS_FREE_LIST_CANARY

#define TOS_FREE_LIST_VERBOSE 1
#undef TOS_FREE_LIST_VERBOSE

[[maybe_unused]] constexpr auto canary_pattern = 0xffffffffffffffff;
struct alignas(16) allocation_header {
#if defined(TOS_FREE_LIST_CANARY)
    size_t pattern = canary_pattern;
#endif
    size_t size;
};

static_assert(sizeof(allocation_header) == 16);

free_header& split(free_header& current, size_t first_size) {
    auto second_ptr = reinterpret_cast<char*>(&current) + first_size;
    return *new (second_ptr) free_header(current.size - first_size);
}

free_list::free_list(tos::span<uint8_t> buffer)
    : m_buffer{buffer} {
    auto root = new (m_buffer.data()) free_header(m_buffer.size());
    m_list.push_back(*root);
}

void* free_list::allocate(size_t size) {
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG("Init", this, m_buffer.data(), m_buffer.size());
#endif

    size = align_nearest_up_pow2(size, 16);
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG(this, size);
#endif

    // If an allocation plus metadata is smaller than a free_header, we must pad it so
    // that we can later free it.
    auto sz = std::max(sizeof(free_header), size + sizeof(allocation_header));

    auto first_fit =
        std::find_if(m_list.begin(), m_list.end(), [sz](const free_header& header) {
            return header.size >= sz;
        });
    if (first_fit == m_list.end()) {
        return nullptr;
    }
    m_list.erase(first_fit);

    if (first_fit->size >= sz + sizeof(free_header)) {
        add_block(split(*first_fit, sz));
    } else {
        sz = first_fit->size;
    }
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG("Before", this, m_used_ctr.get());
#endif
    m_used_ctr.inc(sz);
    m_peak_memory.max(m_used_ctr.get());
    auto header = new (&*first_fit) allocation_header{sz};
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG("After", this, m_used_ctr.get(), header + 1);
#endif
    return std::launder(header + 1);
}

void* free_list::realloc(void* oldptr, size_t size) {
    if (!oldptr) {
        return allocate(size);
    }
    auto header = static_cast<allocation_header*>(oldptr) - 1;
    auto oldsize = header->size;
    if (oldsize - sizeof(allocation_header) >= size) {
        // There is already space for it
        return oldptr;
    }
    auto newptr = allocate(size);
    memcpy(newptr, oldptr, oldsize);
    free(oldptr);
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG("Realloc of", oldptr, (int)oldsize, "->", newptr, (int)size);
#endif
    return newptr;
}

NO_INLINE
void heap_corruption() {
    LOG_ERROR("Heap corruption!");
    while (true)
        ;
}

void free_list::free(void* ptr) {
    auto header = static_cast<allocation_header*>(ptr) - 1;
    auto size = header->size;
#if defined(TOS_FREE_LIST_DEBUG)
    if (ptr < m_buffer.begin() || ptr >= m_buffer.end()) {
        heap_corruption();
    }
#endif
#if defined(TOS_FREE_LIST_VERBOSE)
    LOG("Free",
        ptr,
        m_buffer.begin(),
        m_buffer.end(),
        m_used_ctr.get(),
        size
#if defined(TOS_FREE_LIST_CANARY)
        ,
        (void*)header->pattern
#endif
    );
#endif

#if defined(TOS_FREE_LIST_CANARY)
    if (header->pattern != canary_pattern) {
        heap_corruption();
    }
#endif

    auto new_header = new (header) free_header(size);
    add_block(*new_header);
    m_used_ctr.dec(size);
}

bool is_contiguous(const free_header& first, const free_header& next) {
    auto next_addr = reinterpret_cast<const char*>(&next);
    auto first_addr = reinterpret_cast<const char*>(&first);
    auto first_end = first_addr + first.size;
    return first_end == next_addr;
}

void free_list::add_block(free_header& header) {
    // keep it sorted by address
    auto it = std::find_if(m_list.begin(), m_list.end(), [&header](free_header& extant) {
        return &extant > &header;
    });
    auto inserted = m_list.insert(it, header);
    auto next = inserted;
    ++next;
    if (next == m_list.end()) {
        return;
    }

    if (is_contiguous(*inserted, *next)) {
        inserted->size += next->size;
        m_list.erase(next);
    }

    if (inserted != m_list.begin()) {
        auto prev = inserted;
        --prev;
        if (is_contiguous(*prev, *inserted)) {
            prev->size += inserted->size;
            m_list.erase(inserted);
        }
    }
}
} // namespace tos::memory