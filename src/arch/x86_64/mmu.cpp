#include <tos/debug/log.hpp>
#include <tos/flags.hpp>
#include <tos/paging.hpp>
#include <tos/x86_64/mmu.hpp>

namespace tos::x86_64 {
translation_table& get_current_translation_table() {
    return *reinterpret_cast<translation_table*>(read_cr3());
}

namespace {
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

static_assert(pt_path_for_addr(0) == std::array<int, 4>{0, 0, 0, 0});
static_assert(pt_path_for_addr(1) == std::array<int, 4>{0, 0, 0, 0});
static_assert(pt_path_for_addr(page_size_bytes) == std::array<int, 4>{0, 0, 0, 1});

bool last_level(int level, const table_entry& entry) {
    if (level == 3)
        return true;
    return entry.huge_page();
}

void do_traverse_table_entries(int level,
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
template<size_t N>
expected<void, mmu_errors> recursive_allocate(translation_table& root,
                                              permissions perms,
                                              user_accessible allow_user,
                                              const std::array<int, N>& path,
                                              physical_page_allocator* palloc) {
    if constexpr (N == 1) {
        if (root[path[0]].valid()) {
            tos::debug::error("Page already allocated");
            return unexpected(mmu_errors::already_allocated);
        }

        root[path[0]].zero();
        root[path[0]].noexec(true);

        if (tos::util::is_flag_set(perms, permissions::write)) {
            root[path[0]].writeable(true);
        }

        if (tos::util::is_flag_set(perms, permissions::execute)) {
            root[path[0]].noexec(false);
        }

        if (allow_user == user_accessible::yes) {
            root[path[0]].allow_user(true);
        }

        return {};
    } else {
        if (!root[path[0]].valid()) {
            if (!palloc) {
                tos::debug::error("Need to allocate, but no allocator");
                return unexpected(mmu_errors::page_alloc_fail);
            }

            auto page = palloc->allocate(1);
            if (page == nullptr) {
                tos::debug::error("Page allocator failed");
                return unexpected(mmu_errors::page_alloc_fail);
            }

            auto res = map_region(get_current_translation_table(),
                                  segment{.range = {.base = reinterpret_cast<uintptr_t>(
                                                        palloc->address_of(*page)),
                                                    .size = page_size_bytes},
                                          permissions::read_write},
                                  user_accessible::no,
                                  memory_types::normal,
                                  palloc,
                                  palloc->address_of(*page));

            if (!res) {
                palloc->free({page, 1});
                return unexpected(force_error(res));
            }

            memset(palloc->address_of(*page), 0, page_size_bytes);

            root[path[0]]
                .zero()
                .page_num(palloc->page_num(*page) << page_size_log)
                .valid(true)
                .writeable(true)
                .allow_user(allow_user == user_accessible::yes);
        }

        return recursive_allocate(
            root.table_at(path[0]), perms, allow_user, pop_front(path), palloc);
    }
}
} // namespace

expected<void, mmu_errors> allocate_region(translation_table& root,
                                           const segment& virt_seg,
                                           user_accessible allow_user,
                                           physical_page_allocator* palloc) {
    for (uintptr_t addr = virt_seg.range.base; addr != virt_seg.range.end();
         addr += page_size_bytes) {
        auto path = pt_path_for_addr(addr);
        LOG_TRACE("Address", (void*)addr, path[0], path[1], path[2], path[3]);
        EXPECTED_TRYV(recursive_allocate(root, virt_seg.perms, allow_user, path, palloc));
    }

    return {};
}

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const memory_range& range,
                                         memory_types type,
                                         void* phys_addr) {
    for (uintptr_t addr = range.base; addr != range.end();
         addr += page_size_bytes, phys_addr = (char*)phys_addr + page_size_bytes) {
        auto path = pt_path_for_addr(addr);

        translation_table* table = &root;
        // The last index takes us to the actual page, we'll set that now.
        for (size_t i = 0; i < path.size() - 1; ++i) {
            auto& elem = (*table)[path[i]];
            if (!elem.valid()) {
                return unexpected(mmu_errors::not_allocated);
            }

            table = &table_at(elem);
        }

        (*table)[path.back()].page_num(address_to_page(phys_addr) << page_size_log).valid(true);
        if (type == memory_types::device) {
            // (*table)[path.back()]
            //     .shareable(tos::aarch64::shareable_values::outer)
            //     .mair_index(PT_DEV);
        } else {
            // (*table)[path.back()]
            //     .shareable(tos::aarch64::shareable_values::inner)
            //     .mair_index(PT_MEM);
        }
    }

    return {};
}

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const memory_range& virt_range) {
    for (uintptr_t addr = virt_range.base; addr != virt_range.end();
         addr += page_size_bytes) {
        auto path = pt_path_for_addr(addr);

        translation_table* table = &root;
        // The last index takes us to the actual page, we'll set that now.
        for (size_t i = 0; i < path.size() - 1; ++i) {
            table = &table->table_at(path[i]);
        }

        (*table)[path.back()].valid(false);
    }

    return {};
}

void traverse_table_entries(translation_table& table,
                            function_ref<void(memory_range, table_entry&)> fn) {
    do_traverse_table_entries(0, 0, table, fn);
}

permissions translate_permissions(const table_entry& entry) {
    auto perms = permissions::read;
    if (!entry.readonly()) {
        perms = util::set_flag(perms, permissions::write);
    }
    if (!entry.noexec()) {
        perms = util::set_flag(perms, permissions::execute);
    }
    return perms;
}
} // namespace tos::x86_64