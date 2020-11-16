//
// Created by fatih on 7/12/18.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace tos {
/**
 * This type represents a contiguous region in a memory.
 * The memory in which the addresses in the range may not be addressable directly by the
 * current processor, hence uintptr_t rather than void*.
 */
struct memory_range {
    std::uintptr_t base;
    std::ptrdiff_t size;

    [[nodiscard]] std::uintptr_t end() const {
        return base + size;
    }
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
    return range.base <= addr && range.end() >= addr;
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
} // namespace default_segments
} // namespace tos