#pragma once

#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/x86_64/assembly.hpp>
#include <tos/x86_64/mmu/detail/nested.hpp>
#include <tos/x86_64/mmu/detail/recursive_allocate.hpp>
#include <tos/x86_64/mmu/errors.hpp>
#include <tos/x86_64/mmu/table_entry.hpp>
#include <tos/x86_64/mmu/translation_table.hpp>

namespace tos::x86_64 {
void traverse_table_entries(
    translation_table& table,
    function_ref<void(memory_range vrange, table_entry& entry)> fn);

template<class FnT>
void traverse_table_entries(translation_table& table, FnT&& fn) {
    traverse_table_entries(table, function_ref<void(memory_range, table_entry&)>(fn));
}

expected<void, mmu_errors> allocate_region(translation_table& root,
                                           const segment& virt_seg,
                                           user_accessible allow_user,
                                           physical_page_allocator* palloc);

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const memory_range& range,
                                         memory_types type,
                                         void* phys_addr);

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const memory_range& virt_range);

expected<const table_entry*, mmu_errors> entry_for_address(const translation_table& root,
                                                           uintptr_t virt_addr);

expected<translation_table*, mmu_errors> clone(const translation_table& root,
                                               physical_page_allocator& palloc);

inline expected<void, mmu_errors> map_region(translation_table& root,
                                             const segment& vseg,
                                             user_accessible user_access,
                                             memory_types mem_type,
                                             physical_page_allocator* palloc,
                                             void* phys_base) {
    EXPECTED_TRYV(allocate_region(root, vseg, user_access, palloc));

    EXPECTED_TRYV(mark_resident(root, vseg.range, mem_type, phys_base));

    return {};
}

inline expected<void, mmu_errors>
map_page_ident(translation_table& root,
               physical_page& page,
               physical_page_allocator& palloc,
               permissions perms = permissions::read_write,
               user_accessible user_access = user_accessible::no,
               memory_types mem_type = memory_types::normal) {
    return map_region(root,
                      segment{.range = palloc.range_of(page), .perms = perms},
                      user_access,
                      mem_type,
                      &palloc,
                      palloc.address_of(page));
}

NO_INLINE
inline void tlb_flush() {
    tos::x86_64::write_cr3(tos::x86_64::read_cr3());
}
} // namespace tos::x86_64