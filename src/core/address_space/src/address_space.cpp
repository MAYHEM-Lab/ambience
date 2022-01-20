#include "tos/memory.hpp"
#include <tos/address_space.hpp>
#include <tos/arch.hpp>
#include <tos/backing_object.hpp>
#include <type_traits>

namespace tos {
static_assert(std::is_base_of_v<address_space, cur_arch::address_space>);
cur_arch::address_space* address_space::self() {
    return static_cast<cur_arch::address_space*>(this);
}

const cur_arch::address_space* address_space::self() const {
    return static_cast<const cur_arch::address_space*>(this);
}

mapping* address_space::containing_mapping(virtual_address virt_addr) const {
    for (auto& mapping : m_mappings) {
        if (contains(mapping.vm_segment.range, virt_addr)) {
            return &mapping;
        }
    }

    return nullptr;
}


auto address_space::containing_mapping_range(virtual_range range) const
    -> std::pair<list_it, list_it> {
    auto front = containing_mapping(range.base);

    if (!front) {
        return {m_mappings.end(), m_mappings.end()};
    }

    auto begin = m_mappings.unsafe_find(*front);
    for (auto it = begin; it != m_mappings.end(); ++it) {
        if (contains(it->vm_segment.range, range.end())) {
            return {begin, it};
        }
    }

    return {begin, m_mappings.end()};
}

void address_space::add_mapping(mapping& mapping) {
    auto it = m_mappings.begin();
    for (; it != m_mappings.end(); ++it) {
        if (it->vm_segment.range.base > mapping.vm_segment.range.base) {
            break;
        }
    }
    
    m_mappings.insert(it, mapping);
    mapping.va = self();
}

auto address_space::mappings() const -> const mapping_list_t& {
    return m_mappings;
}

address_space::address_space(const address_space& rhs) {
    for (auto& map : rhs.mappings()) {
        auto our_mapping = new mapping;
        memcpy(our_mapping, &map, sizeof map);
        our_mapping->obj->clone_mapping(map, *our_mapping);
        our_mapping->owned = true;
        add_mapping(*our_mapping);
    }
}

namespace global {
NO_ZERO cur_arch::address_space* cur_as;
}
} // namespace tos