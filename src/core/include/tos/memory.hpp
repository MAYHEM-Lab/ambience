//
// Created by fatih on 7/12/18.
//

#pragma once

#include <cstddef>
#include <tos/utility.hpp>

namespace tos {
/**
 * This type represents a contiguous region in memory.
 */
struct memory_region {
    std::uintptr_t base;
    std::uint32_t size;
};

/**
 * Returns whether the given big memory region entirely contains the given
 * small memory region.
 * @param big memory region to see if it contains the other one
 * @param small memory region to see if it's contained by the other one
 * @return whether the big region contains the small region
 */
constexpr bool contains(const memory_region& big, const memory_region& small) {
    return big.base <= small.base && big.base + big.size >= small.base + small.size;
}
} // namespace tos