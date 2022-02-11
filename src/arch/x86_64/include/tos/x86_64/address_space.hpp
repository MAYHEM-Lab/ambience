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

    result<void> handle_memory_fault(const exception_frame& frame,
                                     virtual_address fault_addr,
                                     bool is_write,
                                     bool is_execute);

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

    physical_address raw_allocate(size_t sz, size_t align);

    // Map the pages into the target.
    bool finalize();

    physical_address traverse_from_address(virtual_address addr) {
        using namespace address_literals;
        return resident_address(*from->m_table, addr).get_or(0_physical);
    }

    virtual_address copy_read_only(virtual_range range_in_from);

    virtual_address map_read_only(virtual_range range_in_from);

    virtual_address map_read_write(virtual_range range_in_from);

    share_page* add_page(int contiguous = 1);

    template<class T, class... Args>
    T* allocate(Args&&... args) {
        auto ptr = raw_allocate(sizeof(T), alignof(T));
        return new (ptr.direct_mapped()) T(std::forward<Args>(args)...);
    }

    ~temporary_share();
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