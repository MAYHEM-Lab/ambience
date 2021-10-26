//
// Created by fatih on 7/12/18.
//

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <tos/intrusive_list.hpp>
#include <tos/intrusive_ptr.hpp>
#include <tos/span.hpp>

namespace tos {
/**
 * This type represents a contiguous region in a memory.
 * The memory in which the addresses in the range may not be addressable directly by the
 * current processor, hence uintptr_t rather than void*.
 */
struct memory_range {
    std::uintptr_t base;
    std::ptrdiff_t size;

    [[nodiscard]] constexpr std::uintptr_t end() const {
        return base + size;
    }

    auto operator<=>(const memory_range&) const noexcept = default;
};

/**
 * Returns whether the given big memory region entirely contains the given
 * small memory region.
 * @param big memory region to see if it contains the other one
 * @param small memory region to see if it's contained by the other one
 * @return whether the big region contains the small region
 */
constexpr bool contains(const memory_range& big, const memory_range& small) {
    return big.base <= small.base && big.end() >= small.end();
}

constexpr bool contains(const memory_range& range, uintptr_t addr) {
    return range.base <= addr && addr < range.end();
}

constexpr std::optional<memory_range> intersection(const memory_range& a,
                                                   const memory_range& b) {
    memory_range res{};
    res.base = std::max(a.base, b.base);
    auto end = std::min(a.end(), b.end());
    if (end <= res.base) {
        return {};
    }
    res.size = end - res.base;
    return res;
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

struct segment {
    memory_range range;
    permissions perms;
};

namespace default_segments {
memory_range image();
memory_range data();
memory_range text();
memory_range rodata();
memory_range bss();
memory_range bss_map();
} // namespace default_segments

struct physical_address {
    uintptr_t addr;

    explicit constexpr physical_address(uintptr_t addr) : addr{addr} {}
    constexpr physical_address(std::nullptr_t) : addr{0} {}

    constexpr operator bool() {
        return addr != 0;
    }

    uintptr_t address() const {
        return addr;
    }

    // Returns a pointer to void to the physical address.
    // Use of the returned pointer assumes the address is mapped direct to the current
    // virtual address space.
    void* direct_mapped() const {
        return reinterpret_cast<void*>(addr);
    }
};
} // namespace tos