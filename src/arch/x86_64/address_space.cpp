#include "tos/memory.hpp"
#include <tos/address_space.hpp>
#include <tos/backing_object.hpp>
#include <tos/x86_64/address_space.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::x86_64 {
expected<bool, mmu_errors>
address_space::handle_memory_fault(const exception_frame& frame, virtual_address fault_addr) {
    auto mapping = containing_mapping(fault_addr);

    if (!mapping) {
        return false;
    }

    memory_fault fault;
    fault.map = mapping;
    fault.virt_addr = fault_addr;

    if (!mapping->obj->handle_memory_fault(fault)) {
        return false;
    }

    invlpg(fault_addr.address());

    return true;
}

expected<address_space, mmu_errors>
address_space::empty(physical_page_allocator& palloc) {
    auto root_page = palloc.allocate(1);
    if (!root_page) {
        return unexpected(mmu_errors::page_alloc_fail);
    }
    map_page_ident(get_current_translation_table(), *root_page, palloc);
    auto root_table = new (palloc.address_of(*root_page).direct_mapped()) translation_table{};
    return address_space{*root_table};
}

void address_space::remove_mapping(mapping& mapping) {
    tos::ensure(mark_nonresident(*m_table, mapping.vm_segment.range));
}

expected<std::unique_ptr<address_space>, mmu_errors>
address_space::clone(physical_page_allocator& palloc) {
    auto cloned_table = EXPECTED_TRY(x86_64::clone(*m_table, palloc));
    return std::unique_ptr<address_space>(
        new address_space(*cloned_table, static_cast<tos::address_space&>(*this)));
}
} // namespace tos::x86_64