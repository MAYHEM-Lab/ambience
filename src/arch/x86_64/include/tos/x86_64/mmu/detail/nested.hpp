#pragma once

#include "tos/memory.hpp"
#include <array>
#include <tos/function_ref.hpp>
#include <tos/paging/level_computation.hpp>
#include <tos/x86_64/mmu/table_entry.hpp>
#include <tos/x86_64/mmu/translation_table.hpp>

namespace tos::x86_64::detail {
constexpr std::array<int, 5> level_bits = {9, 9, 9, 9, 12};

constexpr auto level_masks = compute_level_masks(level_bits);
constexpr auto level_bit_begins = compute_level_bit_begins(level_bits);
constexpr auto level_bit_sums = compute_level_bit_sums(level_bits);

constexpr int index_on_table(int level, uintptr_t address) {
    return (address & level_masks[level]) >> level_bit_begins[level];
}

static_assert(level_bit_sums[4] == 12);
static_assert(level_bit_sums[3] == 21);
static_assert(level_bit_sums[2] == 30);
static_assert(level_bit_sums[1] == 39);
static_assert(level_bit_sums[0] == 48);

static_assert(index_on_table(1, 1024 * 1024 * 1024 - 1) == 0);
static_assert(index_on_table(1, 1024 * 1024 * 1024) == 1);
static_assert(index_on_table(1, 1024 * 1024 * 1024 + 1) == 1);

constexpr std::array<int, std::size(level_bits) - 1>
pt_path_for_addr(uint64_t virt_addr) {
    std::array<int, 4> path;
    for (size_t i = 0; i < path.size(); ++i) {
        path[i] = index_on_table(i, virt_addr);
    }
    return path;
}

constexpr std::array<int, std::size(level_bits) - 1>
pt_path_for_addr(virtual_address addr) {
    return pt_path_for_addr(addr.address());
}

static_assert(pt_path_for_addr(0) == std::array<int, 4>{0, 0, 0, 0});
static_assert(pt_path_for_addr(1) == std::array<int, 4>{0, 0, 0, 0});
static_assert(pt_path_for_addr(page_size_bytes) == std::array<int, 4>{0, 0, 0, 1});

inline bool last_level(int level, const table_entry& entry) {
    if (level == 3)
        return true;
    return entry.huge_page();
}

inline void do_traverse_table_entries(int level,
                                      uintptr_t begin,
                                      translation_table& table,
                                      function_ref<void(memory_range, table_entry&)> fn) {
    for (size_t i = 0; i < table.size(); ++i) {
        auto& entry = table[i];

        if (!entry.valid()) {
            continue;
        }

        auto beg = begin + i * (1ULL << level_bit_sums[level + 1]);

        if (last_level(level, entry)) {
            fn(memory_range{beg, ptrdiff_t(1ULL << level_bit_sums[level + 1])}, entry);
            continue;
        }

        do_traverse_table_entries(level + 1, beg, table_at(entry), fn);
    }
}
} // namespace tos::x86_64::detail