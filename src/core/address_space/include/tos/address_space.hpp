#pragma once

#include <tos/arch.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>

namespace tos {
class address_space {
public:
    explicit address_space(cur_arch::address_space& space);

    mapping* containing_mapping(uintptr_t virt_addr);

    auto do_mapping(tos::mapping& mapping, physical_page_allocator* palloc) {
        auto res = m_backend->allocate_region(mapping, palloc);
        if (res) {
            // Only modify internal state if the allocation succeeds.
            m_mappings.push_back(mapping);
            mapping.va = this;
        }
        return res;
    }

    cur_arch::address_space* m_backend;

private:
    intrusive_list<mapping, through_member<&mapping::va_list>> m_mappings;
};

namespace global {
inline address_space* cur_as;
}
} // namespace tos