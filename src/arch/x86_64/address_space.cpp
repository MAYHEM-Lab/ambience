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

bool temporary_share::finalize() {
    for (auto& page : pages) {
        auto prange = palloc->range_of(page.pages);
        virtual_range vrange = map_at(prange, page.map_at);
        // tos::debug::log(
        //     "map", vrange.base, vrange.end(), prange.base, page.unmap, page.owned);

        bool tried_once = false;

        if (!page.owned) {
            if (to->increment(vrange.base) == 0) {
                goto do_map;
            }
            continue;
        }

    do_map:
        virtual_segment virtseg{.range = vrange,
                                .perms = page.readonly ? permissions::read
                                                       : permissions::read_write};

        auto res = map_region(*to->m_table,
                              virtseg,
                              user_accessible::yes,
                              memory_types::normal,
                              palloc,
                              prange.base);

        if (res) {
            continue;
        }

        auto err = force_error(res);
        if (err == mmu_errors::already_allocated) {
            // tos::debug::log("already allocated");
            auto entry = force_get(entry_for_address(*to->m_table, vrange.base));

            // If the page is already mapped with a user accessible mapping, we cannot
            // fix this here.
            if (entry->allow_user() || tried_once) {
                tos::debug::warn("Tried to map to an existing mapping at",
                                 vrange.base.address(),
                                 vrange.size);
                tos::debug::warn("Tried/existing permissions",
                                 magic_enum::enum_name(virtseg.perms),
                                 magic_enum::enum_name(translate_permissions(*entry)));
                page.unmap = false;
                continue;
            }

            entry->valid(false);
            tried_once = true;
            goto do_map;
        }

        return false;
    }
    return true;
}

temporary_share::~temporary_share() {
    for (auto& page : pages) {
        auto prange = palloc->range_of(page.pages);
        virtual_range vrange = map_at(prange, page.map_at);

        // tos::debug::log(
        //     "unmap", vrange.base, vrange.end(), prange.base, page.unmap,
        //     page.owned);

        if (!page.owned) {
            if (auto remaining = to->decrement(vrange.base); remaining != 0) {
                // tos::debug::log("has more refs", remaining);
                continue;
            }
        }

        if (page.unmap) {
            mark_nonresident(*to->m_table, vrange);
        }

        if (!page.owned) {
            continue;
        }

        mark_nonresident(get_current_translation_table(), vrange);
        palloc->free(page.pages);
    }
}

temporary_share::share_page* temporary_share::add_page(int contiguous) {
    auto alloc = palloc->allocate(contiguous);

    if (!alloc) {
        return nullptr;
    }

    auto pages = span<physical_page>{alloc, size_t(contiguous)};

    auto map_res = map_page_ident(get_current_translation_table(), pages, *palloc);
    if (!map_res) {
        palloc->free(pages);
        return nullptr;
    }

    auto& elem = this->pages.emplace_front();
    elem.owned = true;
    elem.readonly = false;
    elem.unmap = true;
    elem.space = page_size_bytes * contiguous;
    elem.pages = pages;
    elem.map_at = force_get(map_res).base;
    return &elem;
}

virtual_address temporary_share::map_read_write(virtual_range range_in_from) {
    auto page_count = range_in_from.size / page_size_bytes;
    if (page_count > 1) {
        tos::debug::error("Multipage share, buggy");
    }
    auto physical = traverse_from_address(range_in_from.base);
    auto& elem = pages.emplace_front();
    elem.owned = false;
    elem.readonly = false;
    elem.unmap = true;
    elem.space = 0;
    elem.pages = {palloc->info(physical), size_t(page_count)};
    elem.map_at = range_in_from.base;
    return elem.map_at;
}

virtual_address temporary_share::map_read_only(virtual_range range_in_from) {
    if (range_in_from.base.address() % page_size_bytes != 0 ||
        range_in_from.size % page_size_bytes != 0) {
        return copy_read_only(range_in_from);
    }
    auto physical = traverse_from_address(range_in_from.base);
    auto page_count = range_in_from.size / page_size_bytes;
    auto& elem = pages.emplace_front();
    elem.owned = false;
    elem.readonly = true;
    elem.unmap = true;
    elem.space = 0;
    elem.pages = {palloc->info(physical), size_t(page_count)};
    elem.map_at = range_in_from.base;
    return range_in_from.base;
}

virtual_address temporary_share::copy_read_only(virtual_range range_in_from) {
    auto ptr = raw_allocate(range_in_from.size, 1);
    memcpy(ptr.direct_mapped(),
           reinterpret_cast<void*>(range_in_from.base.address()),
           range_in_from.size);
    return virtual_address(ptr.address());
}

physical_address temporary_share::raw_allocate(size_t sz, size_t align) {
    if (pages.empty() || pages.front().space < sz) {
        if (!add_page(align_nearest_up_pow2(sz, page_size_bytes) / page_size_bytes)) {
            return physical_address{0UL};
        }
    }

    auto base = palloc->range_of(pages.front().pages);
    auto res = base.base + (base.size - pages.front().space);
    pages.front().space -= sz;
    return res;
}
} // namespace tos::x86_64