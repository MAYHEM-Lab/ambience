#pragma once

#include "tos/error.hpp"
#include <tos/core/arch_fwd.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>

namespace tos {
enum class address_space_errors {
    no_mapping,
};

TOS_ERROR_ENUM(address_space_errors);

class address_space {
public:
    using mapping_list_t = intrusive_list<mapping, through_member<&mapping::va_list>>;
    using list_it = mapping_list_t::iterator;

    address_space() = default;
    address_space(const address_space& rhs);

    mapping* containing_mapping(virtual_address virt_addr) const;

    // An address range can potentially span multiple mappings.
    // This function returns a range of mappings that contain all the addresses in the
    // given range.
    std::pair<list_it, list_it> containing_mapping_range(virtual_range range) const;

    const mapping_list_t& mappings() const;

protected:
    void add_mapping(mapping& mapping);

    cur_arch::address_space* self();
    const cur_arch::address_space* self() const;

private:
    mapping_list_t m_mappings;
};

namespace global {
extern cur_arch::address_space* cur_as;
}
} // namespace tos