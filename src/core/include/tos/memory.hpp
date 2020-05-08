//
// Created by fatih on 7/12/18.
//

#pragma once

#include <cstddef>
#include <cstdint>

namespace tos {
/**
 * This type represents a contiguous region in memory.
 */
struct memory_region {
    std::uintptr_t base;
    std::uint32_t size;

    [[nodiscard]]
    std::uintptr_t end() const {
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
constexpr bool contains(const memory_region& big, const memory_region& small) {
    return big.base <= small.base && big.base + big.size >= small.base + small.size;
}

enum class permissions : uint8_t {
    none,
    read = 1,
    write = 2,
    execute = 4,
    all = 7
};

namespace flag {
template <class T, std::enable_if_t<std::is_enum_v<T>>* = nullptr>
bool is_set(T elem, T val) {
    using cast_t = std::underlying_type_t<T>;
    return static_cast<cast_t>(elem) & static_cast<cast_t>(val);
}
}
} // namespace tos