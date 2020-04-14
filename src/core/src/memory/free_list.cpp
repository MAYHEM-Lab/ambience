//
// Created by Fatih on 2/17/2020.
//

#include <algorithm>
#include <iostream>
#include <tos/intrusive_list.hpp>
#include <tos/memory/free_list.hpp>

namespace tos::memory {
struct free_header : list_node<free_header> {
    explicit free_header(size_t sz)
        : size(sz) {
    }
    size_t size;
};

struct alignas(16) allocation_header {
    size_t size;
};

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
    if (size % 16 != 0) {
        size += 16 - size % 16;
    }
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

    m_used += sz;
    auto header = new (&*first_fit) allocation_header{sz};
    return header + 1;
}

void free_list::free(void* ptr) {
    auto header = static_cast<allocation_header*>(ptr) - 1;
    auto size = header->size;
    auto new_header = new (header) free_header(size);
    add_block(*new_header);
    m_used -= size;
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