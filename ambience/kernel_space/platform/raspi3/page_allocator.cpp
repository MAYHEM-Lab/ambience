#include "private.hpp"
#include "tos/memory.hpp"
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>
#include <tos/paging/physical_page_allocator.hpp>

tos::physical_page_allocator* initialize_page_allocator() {
    auto& level0_table = tos::cur_arch::get_current_translation_table();

    using namespace tos::address_literals;
    auto op_res = tos::aarch64::allocate_region(
        level0_table,
        identity_map(tos::physical_range{4096_physical, 4096 * 5}),
        nullptr);
    if (!op_res) {
        LOG_ERROR("Could not allocate ...");
    }

    op_res = tos::aarch64::mark_resident(
        level0_table,
        identity_map(tos::physical_segment{{4096_physical, 4096 * 5},
                                           tos::permissions::read_write}),
        tos::memory_types::normal,
        tos::user_accessible::no,
        4096_physical);
    if (!op_res) {
        LOG_ERROR("Could not mark resident ...");
    }

    tos::aarch64::tlb_invalidate_all();

    auto palloc = new (reinterpret_cast<void*>(4096)) tos::physical_page_allocator(1024);

    tos::aarch64::traverse_table_entries(
        level0_table, [&](tos::memory_range range, tos::aarch64::table_entry& entry) {
            LOG_TRACE(
                "Making [", (void*)range.base, ",", (void*)range.end(), "] unavailable");

            entry.allow_user(true);
            palloc->mark_unavailable({tos::physical_address(range.base), range.size});
        });

    palloc->mark_unavailable({0_physical, 4096});
    tos::aarch64::tlb_invalidate_all();

    return palloc;
}
