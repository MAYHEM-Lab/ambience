#include "private.hpp"
#include <tos/arch.hpp>
#include <tos/debug/log.hpp>
#include <tos/paging/physical_page_allocator.hpp>

tos::physical_page_allocator* initialize_page_allocator() {
    auto& level0_table = tos::cur_arch::get_current_translation_table();

    auto op_res = tos::aarch64::allocate_region(
        level0_table,
        tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
        tos::user_accessible::no,
        nullptr);
    if (!op_res) {
        LOG_ERROR("Could not allocate ...");
    }

    op_res = tos::aarch64::mark_resident(
        level0_table,
        tos::segment{{4096, 4096 * 5}, tos::permissions::read_write},
        tos::memory_types::normal,
        (void*)4096);
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
            palloc->mark_unavailable(range);
        });

    palloc->mark_unavailable({0, 4096});
    tos::aarch64::tlb_invalidate_all();

    return palloc;
}
