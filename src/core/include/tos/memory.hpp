//
// Created by fatih on 7/12/18.
//

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tos/span.hpp>

namespace tos {
// This type represents an address in a virtual address space.
// That address space may not be the current one, so dereferencing this virtual
// address directly is not safe!
struct virtual_address {
    uintptr_t addr;

    virtual_address() = default;
    explicit constexpr virtual_address(uintptr_t addr)
        : addr{addr} {
    }
    constexpr virtual_address(std::nullptr_t)
        : addr{0} {
    }

    constexpr explicit operator bool() {
        return addr != 0;
    }

    [[nodiscard]] constexpr uintptr_t address() const {
        return addr;
    }

    constexpr friend virtual_address operator+(const virtual_address& base,
                                               std::ptrdiff_t diff) {
        auto copy = base;
        copy += diff;
        return copy;
    }

    constexpr friend virtual_address& operator+=(virtual_address& base,
                                                 std::ptrdiff_t diff) {
        base.addr += diff;
        return base;
    }

    constexpr friend std::ptrdiff_t operator-(const virtual_address& left,
                                              const virtual_address& right) {
        return left.address() - right.address();
    }

    constexpr auto operator<=>(const virtual_address&) const noexcept = default;
};

// This type models a memory address in physical memory.
// Objects of this type often are not directly dereference-able,
// hence, this type does not implement a pointer interface.
// A dereferenceable pointer can be acquired by going through an
// address space.
// In case of no virtual memory or direct-mapped virtual address
// spaces, this type provides a `direct_mapped` member.
struct physical_address {
    uintptr_t addr;

    physical_address() = default;
    explicit constexpr physical_address(uintptr_t addr)
        : addr{addr} {
    }
    constexpr physical_address(std::nullptr_t)
        : addr{0} {
    }

    constexpr explicit operator bool() {
        return addr != 0;
    }

    [[nodiscard]] constexpr uintptr_t address() const {
        return addr;
    }

    constexpr friend physical_address operator+(physical_address base,
                                                std::ptrdiff_t diff) {
        return physical_address{base.address() + diff};
    }

    constexpr friend std::ptrdiff_t operator-(const physical_address& left,
                                              const physical_address& right) {
        return left.address() - right.address();
    }

    constexpr auto operator<=>(const physical_address&) const noexcept = default;

    friend virtual_address identity_map(const physical_address& addr) {
        return virtual_address(addr.address());
    }

    // Returns a pointer to void to the physical address.
    // Use of the returned pointer assumes the address is mapped direct to the current
    // virtual address space.
    [[nodiscard]] void* direct_mapped() const {
        return reinterpret_cast<void*>(addr);
    }
};

template<class BaseT, class SizeT>
struct mem_range {
    BaseT base;
    SizeT size;

    [[nodiscard]] constexpr auto end() const {
        return base + size;
    }

    constexpr auto operator<=>(const mem_range&) const noexcept = default;

    /**
     * Returns whether the given big memory region entirely contains the given
     * small memory region.
     * @param big memory region to see if it contains the other one
     * @param small memory region to see if it's contained by the other one
     * @return whether the big region contains the small region
     */
    friend constexpr bool contains(const mem_range& big, const mem_range& small) {
        return big.base <= small.base && big.end() >= small.end();
    }

    friend constexpr bool contains(const mem_range& range, BaseT addr) {
        return range.base <= addr && addr < range.end();
    }

    friend constexpr std::optional<mem_range> intersection(const mem_range& a,
                                                           const mem_range& b) {
        mem_range res{};
        res.base = std::max(a.base, b.base);
        auto end = std::min(a.end(), b.end());
        if (end <= res.base) {
            return {};
        }
        res.size = end - res.base;
        return res;
    }
};

namespace address_literals {
constexpr physical_address operator""_physical(unsigned long long addr) {
    return physical_address(addr);
}

constexpr virtual_address operator""_virtual(unsigned long long addr) {
    return virtual_address(addr);
}
} // namespace address_literals

/**
 * This type represents a contiguous region in a memory.
 * The memory in which the addresses in the range may not be addressable directly by the
 * current processor, hence uintptr_t rather than void*.
 */
using memory_range = mem_range<std::uintptr_t, std::ptrdiff_t>;

using physical_range = mem_range<physical_address, std::ptrdiff_t>;
using virtual_range = mem_range<virtual_address, std::ptrdiff_t>;

inline virtual_range identity_map(const physical_range& range) {
    return virtual_range{.base = identity_map(range.base), .size = range.size};
}

inline virtual_range map_at(const physical_range& range, virtual_address at) {
    return virtual_range{.base = at, .size = range.size};
}

template<class BaseT, class SizeT>
constexpr std::enable_if_t<!std::is_same_v<mem_range<BaseT, SizeT>, memory_range>,
                           memory_range>
to_memory_range(const mem_range<BaseT, SizeT>& range) {
    return memory_range{.base = range.base.address(), .size = range.size};
}

enum class permissions : uint8_t
{
    none,
    read = 1,
    write = 2,
    read_write = 3,
    execute = 4,
    read_execute = 5,
    all = 7
};

enum class memory_types
{
    // RAM, cachable
    normal = 1,

    // MMIO etc, not cached
    device = 2
};

enum class user_accessible : uint8_t
{
    no = 0,
    yes = 1
};

template<class RangeType>
struct basic_segment {
    RangeType range;
    permissions perms;
};

using segment = basic_segment<memory_range>;
using physical_segment = basic_segment<physical_range>;
using virtual_segment = basic_segment<virtual_range>;


inline virtual_segment identity_map(const physical_segment& range) {
    return virtual_segment{.range = identity_map(range.range), .perms = range.perms};
}

namespace default_segments {
physical_range image();
physical_range data();
physical_range text();
physical_range rodata();
physical_range bss();

// We support NOZERO sections which store objects with static lifetimes, but do not get
// zero initialized, and therefore do not end up in .bss
// So we use the bss segment to zero initialize, but bss_map to map all of the global
// variables.
physical_range bss_map();
} // namespace default_segments
} // namespace tos