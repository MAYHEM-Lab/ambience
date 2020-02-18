//
// Created by Fatih on 2/17/2020.
//

#include <tos/memory/free_list.hpp>
#include <tos/intrusive_list.hpp>
#include <algorithm>

namespace tos::memory {
struct free_header : list_node<free_header> {
    size_t size;

    void* ptr() {
        return this;
    }
};

struct allocation_header {
    size_t size;
};

std::pair<free_header*, free_header*> split(free_header& current, size_t first_size) {
    auto second_ptr = static_cast<char*>(current.ptr()) + first_size;
    auto second = new (second_ptr) free_header;
    second->size = current.size - first_size;
    current.size = first_size;
    return { &current, second };
}

free_list::free_list(tos::span<uint8_t> buffer) : m_buffer{buffer} {
    auto root = new (buffer.data()) free_header;
    root->size = buffer.size();
    m_list.push_back(*root);
}

void* free_list::allocate(size_t size) {
    auto first_fit = std::find_if(m_list.begin(), m_list.end(), [size](const free_header& header) {
        return header.size >= size + sizeof(allocation_header);
    });
    if (first_fit == m_list.end()) {
        return nullptr;
    }
    m_list.erase(first_fit);

    if (first_fit->size >= size + sizeof(allocation_header) + sizeof(free_header)) {
        auto [first, second] = split(*first_fit, size + sizeof(allocation_header));
        m_list.push_back(*second);
        auto header = new (first) allocation_header{size};
        return header + 1;
    }

    auto header = new (&*first_fit) allocation_header{size};
    return header + 1;
}

void free_list::free(void* ptr) {
}
}