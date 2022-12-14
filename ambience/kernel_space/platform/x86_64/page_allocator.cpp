#include "private.hpp"
#include "tos/memory.hpp"
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>

tos::expected<tos::physical_page_allocator*, tos::cur_arch::mmu_errors>
initialize_page_allocator() {
    auto& root_table = tos::cur_arch::get_current_translation_table();

    constexpr auto page_num = 2048 * 80;
    auto vmem_end = tos::default_segments::image().end().direct_mapped();

    LOG("Image ends at", vmem_end);

    auto allocator_space =
        tos::align_nearest_up_pow2(tos::physical_page_allocator::size_for_pages(page_num),
                                   tos::cur_arch::page_size_bytes);
    LOG("Physpage allocator would need", allocator_space, "bytes");

    auto allocator_segment = tos::physical_segment{
        tos::physical_range{tos::physical_address(uintptr_t(vmem_end)),
                            ptrdiff_t(allocator_space)},
        tos::permissions::read_write};
    LOG("Call allocate", allocator_space, "bytes");

    EXPECTED_TRYV(tos::cur_arch::allocate_region(
        root_table, identity_map(allocator_segment).range, nullptr));
    LOG("Allocated", allocator_space, "bytes");

    EXPECTED_TRYV(tos::cur_arch::mark_resident(
        root_table,
        identity_map(allocator_segment),
        tos::memory_types::normal,
        tos::user_accessible::no,
        tos::physical_address{reinterpret_cast<uintptr_t>(vmem_end)}));
    LOG("Marked resident", allocator_space, "bytes");

    auto palloc = new (vmem_end) tos::physical_page_allocator(page_num);

    using namespace tos::address_literals;

    palloc->mark_unavailable(tos::default_segments::image());
    palloc->mark_unavailable({0_physical, tos::cur_arch::page_size_bytes});
    palloc->mark_unavailable({0x00080000_physical, 0x000FFFFF - 0x00080000});
    LOG("Available:", palloc, palloc->remaining_page_count());

    return palloc;
}
