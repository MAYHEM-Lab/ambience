#pragma once

#include "tos/x86_64/mmu/common.hpp"
#include <forward_list>
#include <tos/address_space.hpp>
#include <tos/function_ref.hpp>
#include <tos/mapping.hpp>
#include <tos/memory.hpp>
#include <tos/quik.hpp>
#include <tos/x86_64/exception.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::x86_64 {
struct address_space final : tos::address_space {
    address_space(tos::detail::dangerous_tag){};

    expected<void, mmu_errors> allocate_region(mapping& mapping,
                                               physical_page_allocator* palloc) {
        Assert(mapping.vm_segment.range.base % page_size_bytes == 0);
        Assert(mapping.vm_segment.range.size % page_size_bytes == 0);
        EXPECTED_TRYV(x86_64::allocate_region(
            *m_table, mapping.vm_segment, mapping.allow_user, palloc));
        return {};
    }

    auto do_mapping(mapping& mapping, physical_page_allocator* arg) {
        auto res = allocate_region(mapping, arg);
        if (res) {
            // Only modify internal state if the allocation succeeds.
            add_mapping(mapping);
        }
        return res;
    }

    void remove_mapping(mapping& mapping);

    expected<void, mmu_errors>
    mark_resident(mapping& mapping, memory_range subrange, void* phys_addr) {
        return x86_64::mark_resident(*m_table, subrange, mapping.mem_type, phys_addr);
    }

    expected<bool, mmu_errors> handle_memory_fault(const exception_frame& frame,
                                                   uintptr_t fault_addr);

    static memory_range containing_fragment(memory_range range) {
        range.size = align_nearest_up_pow2(range.size, page_size_bytes);
        range.base = align_nearest_down_pow2(range.base, page_size_bytes);
        return range;
    }

    static address_space adopt(translation_table& table) {
        return address_space(table);
    }

    // The cloned address space is owned unlike empty or adopted address spaces as it
    // potentially contains mappings that point back to it.
    expected<std::unique_ptr<address_space>, mmu_errors>
    clone(physical_page_allocator& palloc);

    static expected<address_space, mmu_errors> empty(physical_page_allocator& palloc);

    friend void activate(address_space& to_activate) {
        tos::global::cur_as = &to_activate;
        write_cr3(reinterpret_cast<uint64_t>(to_activate.m_table));
    }

    translation_table* m_table;

private:
    template<class... Ts>
    explicit address_space(translation_table& adopt, Ts&&... base)
        : tos::address_space(std::forward<Ts>(base)...)
        , m_table(&adopt) {
    }
};

struct temporary_share : quik::share_base {
    temporary_share() = default;
    temporary_share(temporary_share&&) noexcept = default;

    //    tos::x86_64::address_space* from;
    tos::x86_64::address_space* to;
    physical_page_allocator* palloc;

    struct share_page {
        physical_page* page;
        bool owned : 1;
        uint16_t space : 13;
    };
    std::forward_list<share_page> pages;

    void* raw_allocate(size_t sz, size_t align) {
        if (sz >= page_size_bytes) {
            return nullptr;
        }
        if (pages.empty() || pages.front().space < sz) {
            add_page();
        }
        auto base = palloc->address_of(*pages.front().page);
        auto res =
            static_cast<std::byte*>(base) + (page_size_bytes - pages.front().space);
        pages.front().space -= sz;
        return res;
    }

    // Map the pages into the target.
    void finalize() {
        for (auto& page : pages) {
            //            tos::debug::log("Mapping page", palloc->address_of(*page.page));
            map_page_ident(*to->m_table,
                           *page.page,
                           *palloc,
                           page.owned ? permissions::read_write : permissions::read,
                           user_accessible::yes);
        }
    }

    share_page* map_read_only(uintptr_t page_base_address_in_from) {
        auto& elem = pages.emplace_front();
        elem.owned = false;
        elem.space = 0;
        elem.page = palloc->info(reinterpret_cast<void*>(page_base_address_in_from));
        return &elem;
    }

    share_page* add_page() {
        auto alloc = palloc->allocate(1);

        if (!alloc) {
            return nullptr;
        }

        if (!map_page_ident(get_current_translation_table(), *alloc, *palloc)) {
            palloc->free({alloc, 1});
            return nullptr;
        }

        auto& elem = pages.emplace_front();
        elem.owned = true;
        elem.space = page_size_bytes;
        elem.page = alloc;
        return &elem;
    }

    template<class T, class... Args>
    T* allocate(Args&&... args) {
        auto ptr = raw_allocate(sizeof(T), alignof(T));
        return new (ptr) T(std::forward<Args>(args)...);
    }

    ~temporary_share() {
        for (auto& page : pages) {
            mark_nonresident(*to->m_table, palloc->range_of(*page.page));
            if (!page.owned) {
                continue;
            }
            mark_nonresident(get_current_translation_table(),
                             palloc->range_of(*page.page));
            palloc->free({page.page, 1});
        }
    }
};

template<class... Ts>
struct typed_share : temporary_share {
    std::tuple<Ts...> ptrs;
    void* get_tuple_ptr() override {
        return &ptrs;
    }
};

template<class... DataPtrTs>
typed_share<DataPtrTs...> create_share(tos::cur_arch::address_space& from,
                                       tos::cur_arch::address_space& to,
                                       physical_page_allocator& palloc,
                                       const std::tuple<DataPtrTs...>& in_ptrs) {
    typed_share<DataPtrTs...> res;
    res.to = &to;
    res.palloc = &palloc;
    quik::perform_share(res, in_ptrs);
    res.finalize();
    return res;
}
} // namespace tos::x86_64