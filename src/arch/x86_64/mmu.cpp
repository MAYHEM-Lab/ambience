#include "tos/expected.hpp"
#include "tos/memory.hpp"
#include "tos/x86_64/mmu/common.hpp"
#include "tos/x86_64/mmu/errors.hpp"
#include "tos/x86_64/mmu/table_entry.hpp"
#include "tos/x86_64/mmu/translation_table.hpp"
#include <tos/debug/log.hpp>
#include <tos/flags.hpp>
#include <tos/paging.hpp>
#include <tos/x86_64/mmu.hpp>
#include <tos/x86_64/mmu/detail/nested.hpp>

namespace tos::x86_64 {
translation_table& get_current_translation_table() {
    return *reinterpret_cast<translation_table*>(read_cr3());
}

using namespace tos::x86_64::detail;

namespace {
template<size_t N>
constexpr expected<void, mmu_errors> recursive_allocate(translation_table& root,
                                                        permissions perms,
                                                        user_accessible allow_user,
                                                        const std::array<int, N>& path,
                                                        physical_page_allocator* palloc) {
    if constexpr (N == 1) {
        if (root[path[0]].valid()) {
            //            tos::debug::error("Page already allocated");
            return unexpected(mmu_errors::already_allocated);
        }

        root[path[0]].zero();
        root[path[0]].noexec(true);

        if (tos::util::is_flag_set(perms, permissions::write)) {
            root[path[0]].writeable(true);
        }

        if (tos::util::is_flag_set(perms, permissions::execute)) {
            root[path[0]].noexec(false);
        }

        if (allow_user == user_accessible::yes) {
            root[path[0]].allow_user(true);
        }

        return {};
    } else {
        if (!root[path[0]].valid()) [[unlikely]] {
            if (!palloc) {
                tos::debug::error("Need to allocate, but no allocator");
                return unexpected(mmu_errors::no_allocator);
            }

            auto page = palloc->allocate(1);
            if (!page) {
                tos::debug::error("Page allocator failed");
                return unexpected(mmu_errors::page_alloc_fail);
            }

            auto res = map_page_ident(get_current_translation_table(), *page, *palloc);

            if (!res) {
                palloc->free({page, 1});
                return unexpected(force_error(res));
            }

            memset(palloc->address_of(*page).direct_mapped(), 0, page_size_bytes);

            root[path[0]]
                .zero()
                .page_num(palloc->page_num(*page) << page_size_log)
                .valid(true)
                .writeable(true)
                .allow_user(true);
        }

        return recursive_allocate(
            root.table_at(path[0]), perms, allow_user, pop_front(path), palloc);
    }
}
} // namespace

expected<void, mmu_errors> allocate_region(translation_table& root,
                                           const virtual_segment& virt_seg,
                                           user_accessible allow_user,
                                           physical_page_allocator* palloc) {
    for (auto addr = virt_seg.range.base; addr != virt_seg.range.end();
         addr += page_size_bytes) {
        auto path = detail::pt_path_for_addr(addr);
        //        LOG_TRACE("Address", (void*)addr, path[0], path[1], path[2], path[3]);
        EXPECTED_TRYV(recursive_allocate(root, virt_seg.perms, allow_user, path, palloc));
    }

    return {};
}

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const virtual_range& range,
                                         memory_types type,
                                         physical_address phys_addr) {
    for (auto addr = range.base; addr != range.end();
         addr += page_size_bytes, phys_addr += page_size_bytes) {
        auto path = pt_path_for_addr(addr);

        translation_table* table = &root;
        // The last index takes us to the actual page, we'll set that now.
        for (size_t i = 0; i < path.size() - 1; ++i) {
            auto& elem = (*table)[path[i]];
            if (!elem.valid()) {
                return unexpected(mmu_errors::not_allocated);
            }

            table = &table_at(elem);
        }

        (*table)[path.back()]
            .page_num(address_to_page(phys_addr) << page_size_log)
            .valid(true);
        if (type == memory_types::device) {
            // (*table)[path.back()]
            //     .shareable(tos::aarch64::shareable_values::outer)
            //     .mair_index(PT_DEV);
        } else {
            // (*table)[path.back()]
            //     .shareable(tos::aarch64::shareable_values::inner)
            //     .mair_index(PT_MEM);
        }
    }

    return {};
}

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const virtual_range& virt_range) {
    for (auto addr = virt_range.base; addr != virt_range.end(); addr += page_size_bytes) {
        auto path = pt_path_for_addr(addr);

        translation_table* table = &root;
        // The last index takes us to the actual page, we'll set that now.
        for (size_t i = 0; i < path.size() - 1; ++i) {
            table = &table->table_at(path[i]);
        }

        (*table)[path.back()].valid(false);
    }

    return {};
}

void traverse_table_entries(translation_table& table,
                            function_ref<void(memory_range, table_entry&)> fn) {
    do_traverse_table_entries(0, 0, table, fn);
}

permissions translate_permissions(const table_entry& entry) {
    auto perms = permissions::read;
    if (!entry.readonly()) {
        perms = util::set_flag(perms, permissions::write);
    }
    if (!entry.noexec()) {
        perms = util::set_flag(perms, permissions::execute);
    }
    return perms;
}

namespace {
expected<translation_table*, mmu_errors>
do_clone(const translation_table& root, physical_page_allocator& palloc, int level) {
    auto page_res = palloc.allocate(1);
    if (!page_res) {
        return unexpected(mmu_errors::page_alloc_fail);
    }

    EXPECTED_TRYV(map_page_ident(get_current_translation_table(), *page_res, palloc));

    auto tbl = new (palloc.address_of(*page_res).direct_mapped()) translation_table(root);

    for (auto& entry : *tbl) {
        if (!entry.valid()) {
            continue;
        }

        if (last_level(level, entry)) {
            // Not a table, nothing to do
            continue;
        }

        auto clone_res = do_clone(table_at(entry), palloc, level + 1);
        if (!clone_res) {
            tos::debug::error("Clone failed");
            return clone_res;
        }
        entry.page_num(address_to_page(force_get(clone_res)) << page_size_log);
    }
    return tbl;
}
} // namespace

expected<table_entry*, mmu_errors> entry_for_address(translation_table& root,
                                                     virtual_address virt_addr) {
    auto path = pt_path_for_addr(virt_addr);
    translation_table* table = &root;

    for (size_t i = 0; i < path.size() - 1; ++i) {
        auto& el = (*table)[path[i]];
        if (!el.valid()) {
            return unexpected(mmu_errors::not_allocated);
        }

        table = &table->table_at(path[i]);
    }

    auto& el = (*table)[path.back()];
    if (!el.valid()) {
        return unexpected(mmu_errors::not_allocated);
    }

    return &(*table)[path.back()];
}

expected<translation_table*, mmu_errors> clone(const translation_table& root,
                                               physical_page_allocator& palloc) {
    // For every entry in a translation table, we copy it if it's a page and clone
    // if it is a table.
    return do_clone(root, palloc, 0);
}

expected<physical_address, mmu_errors> resident_address(const translation_table& root,
                                                        virtual_address addr) {
    auto path = pt_path_for_addr(addr);

    const translation_table* table = &root;
    // The last index takes us to the actual page, we'll set that now.
    for (size_t i = 0; i < path.size() - 1; ++i) {
        auto& elem = (*table)[path[i]];
        if (!elem.valid()) {
            return unexpected(mmu_errors::not_allocated);
        }

        table = &table_at(elem);
    }

    if (!(*table)[path.back()].valid()) {
        return unexpected(mmu_errors::not_allocated);
    }

    auto num = (*table)[path.back()].page_num();
    return physical_address(page_to_address(num));
}
} // namespace tos::x86_64