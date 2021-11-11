#pragma once

#include <tos/core/arch_fwd.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos {
class address_space {
public:
    address_space() = default;
    address_space(const address_space& rhs);

    mapping* containing_mapping(virtual_address virt_addr);

    const auto& mappings() const;

protected:
    void add_mapping(mapping& mapping);

    cur_arch::address_space* self();
    const cur_arch::address_space* self() const;

private:
    intrusive_list<mapping, through_member<&mapping::va_list>> m_mappings;
};

namespace global {
extern cur_arch::address_space* cur_as;
}
} // namespace tos