#pragma once

#include "magic_enum.hpp"
#include "tos/paging/physical_page_allocator.hpp"
#include "tos/utility.hpp"
#include "tos/x86_64/mmu/common.hpp"
#include "tos/x86_64/mmu/errors.hpp"
#include <forward_list>
#include <map>
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
        Assert(mapping.vm_segment.range.base.address() % page_size_bytes == 0);
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
    mark_resident(mapping& mapping, virtual_range subrange, physical_address phys_addr) {
        return x86_64::mark_resident(*m_table, subrange, mapping.mem_type, phys_addr);
    }

    expected<bool, mmu_errors> handle_memory_fault(const exception_frame& frame,
                                                   virtual_address fault_addr);

    static virtual_range containing_fragment(virtual_range range) {
        range.size = align_nearest_up_pow2(range.size, page_size_bytes);
        range.base = virtual_address(
            align_nearest_down_pow2(range.base.address(), page_size_bytes));
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
    std::map<virtual_address, int> mapped;
    std::pair<virtual_address, int> cache{};

    // Returns the previous value before increment
    int increment(virtual_address addr) {
        if (cache.first == addr) {
            return cache.second++;
        }

        auto tree_it = mapped.find(addr);
        if (tree_it != mapped.end()) {
            return tree_it->second++;
        }

        // Neither cached, nor in the tree, we'll insert the cache into the tree and
        // make this element the cache.
        // tos::debug::log("inserting", cache.first, ",", cache.second, "for", addr);
        mapped.insert(cache);
        cache = {addr, 1};
        return 0;
    }

    // Returns the current value after decrement
    int decrement(virtual_address addr) {
        if (cache.first == addr) {
            return --cache.second;
        }

        auto it = mapped.find(addr);
        return --it->second;
    }

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

    tos::x86_64::address_space* from;
    tos::x86_64::address_space* to;
    physical_page_allocator* palloc;

    struct share_page {
        virtual_address map_at;
        span<physical_page> pages;
        uint32_t space : 24;
        bool owned : 1;
        bool readonly : 1;
        bool unmap : 1;
    };
    std::forward_list<share_page> pages;

    physical_address raw_allocate(size_t sz, size_t align) {
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

    // Map the pages into the target.
    bool finalize() {
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
                auto entry =
                    force_get(entry_for_address(*to->m_table, vrange.base));

                // If the page is already mapped with a user accessible mapping, we cannot
                // fix this here.
                if (entry->allow_user() || tried_once) {
                    tos::debug::warn("Tried to map to an existing mapping at",
                                     vrange.base.address(),
                                     vrange.size);
                    tos::debug::warn(
                        "Tried/existing permissions",
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

    physical_address traverse_from_address(virtual_address addr) {
        using namespace address_literals;
        return resident_address(*from->m_table, addr).get_or(0_physical);
    }

    virtual_address copy_read_only(virtual_range range_in_from) {
        auto ptr = raw_allocate(range_in_from.size, 1);
        memcpy(ptr.direct_mapped(),
               reinterpret_cast<void*>(range_in_from.base.address()),
               range_in_from.size);
        return virtual_address(ptr.address());
    }

    virtual_address map_read_only(virtual_range range_in_from) {
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

    virtual_address map_read_write(virtual_range range_in_from) {
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

    share_page* add_page(int contiguous = 1) {
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

    template<class T, class... Args>
    T* allocate(Args&&... args) {
        auto ptr = raw_allocate(sizeof(T), alignof(T));
        return new (ptr.direct_mapped()) T(std::forward<Args>(args)...);
    }

    ~temporary_share() {
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
};

template<class... Ts>
struct typed_share : temporary_share {
    std::tuple<Ts...>* ptrs;
    void* get_tuple_ptr() override {
        return ptrs;
    }
};

template<class... DataPtrTs>
typed_share<DataPtrTs...> create_share(tos::cur_arch::address_space& from,
                                       tos::cur_arch::address_space& to,
                                       const std::tuple<DataPtrTs...>& in_ptrs) {
    typed_share<DataPtrTs...> res;
    res.from = &from;
    res.to = &to;
    res.palloc = physical_page_allocator::instance();
    res.ptrs = quik::perform_share(res, in_ptrs);
    res.finalize();
    return res;
}
} // namespace tos::x86_64