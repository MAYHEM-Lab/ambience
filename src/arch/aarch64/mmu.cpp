#include <numeric>
#include <tos/aarch64/assembly.hpp>
#include <tos/aarch64/mmu.hpp>
#include <tos/aarch64/semihosting.hpp>
#include <tos/debug/log.hpp>
#include <tos/flags.hpp>
#include <tos/paging.hpp>
#include <tos/span.hpp>

namespace tos::aarch64 {
translation_table& get_current_translation_table() {
    return *reinterpret_cast<translation_table*>(get_ttbr0_el1());
}

translation_table& set_current_translation_table(translation_table& table) {
    auto& old = get_current_translation_table();
    semihosting::write0("Setting page table... ");
    tos::aarch64::semihosting::write0(
        tos::itoa(reinterpret_cast<uintptr_t>(&old), 16).data());
    tos::aarch64::semihosting::write0(" -> ");
    tos::aarch64::semihosting::write0(
        tos::itoa(reinterpret_cast<uintptr_t>(&table), 16).data());
    tos::aarch64::semihosting::write0("\n");

    set_ttbr0_el1(&table);
    tlb_invalidate_all();
    return old;
}

namespace {
// defined in MAIR register
static constexpr auto PT_MEM = 0; // normal memory
static constexpr auto PT_DEV = 1; // device MMIO

constexpr std::array<int, 4> level_bits = {9, 9, 9, 12};

constexpr auto level_masks = compute_level_masks(level_bits);
constexpr auto level_bit_begins = compute_level_bit_begins(level_bits);
constexpr auto level_bit_sums = compute_level_bit_sums(level_bits);

constexpr int index_on_table(int level, uintptr_t address) {
    return (address & level_masks[level]) >> level_bit_begins[level];
}

static_assert(level_bit_sums[3] == 12);
static_assert(level_bit_sums[2] == 21);
static_assert(level_bit_sums[1] == 30);
static_assert(level_bit_sums[0] == 39);

static_assert(index_on_table(0, 1024 * 1024 * 1024 - 1) == 0);
static_assert(index_on_table(0, 1024 * 1024 * 1024) == 1);
static_assert(index_on_table(0, 1024 * 1024 * 1024 + 1) == 1);

constexpr std::array<int, std::size(level_bits) - 1>
pt_path_for_addr(uint64_t virt_addr) {
    std::array<int, 3> path;
    for (size_t i = 0; i < path.size(); ++i) {
        path[i] = index_on_table(i, virt_addr);
    }
    return path;
}

static_assert(pt_path_for_addr(0) == std::array<int, 3>{0, 0, 0});
static_assert(pt_path_for_addr(1) == std::array<int, 3>{0, 0, 0});
static_assert(pt_path_for_addr(4096) == std::array<int, 3>{0, 0, 1});

template<size_t N>
expected<void, mmu_errors> recursive_allocate(translation_table& root,
                                              permissions perms,
                                              user_accessible allow_user,
                                              const std::array<int, N>& path,
                                              physical_page_allocator* palloc) {
    if constexpr (N == 1) {
        if (root[path[0]].valid()) {
            return unexpected(mmu_errors::already_allocated);
        }

        root[path[0]].zero().page(true).accessed(true);

        if (!tos::util::is_flag_set(perms, permissions::write)) {
            root[path[0]].readonly(true);
        }

        if (!tos::util::is_flag_set(perms, permissions::execute)) {
            root[path[0]].noexec(true);
        }

        if (allow_user == user_accessible::yes) {
            root[path[0]].allow_user(true);
        }

        return {};
    } else {
        if (!root[path[0]].valid()) {
            if (!palloc) {
                return unexpected(mmu_errors::page_alloc_fail);
            }

            auto page = palloc->allocate(1);
            if (page == nullptr) {
                return unexpected(mmu_errors::page_alloc_fail);
            }

            root[path[0]]
                .zero()
                .page_num(palloc->page_num(*page))
                .valid(true)
                .page(true)
                .accessed(true)
                .allow_user(true)
                .shareable(tos::aarch64::shareable_values::inner)
                .mair_index(PT_MEM);
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
         addr += 4096) {
        auto path = pt_path_for_addr(addr);

        EXPECTED_TRYV(recursive_allocate(root, virt_seg.perms, allow_user, path, palloc));
    }

    return {};
}

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const segment& virt_seg,
                                         memory_types type,
                                         void* phys_addr) {
    for (uintptr_t addr = virt_seg.range.base; addr != virt_seg.range.end();
         addr += 4096, phys_addr = (char*)phys_addr + 4096) {
        auto path = pt_path_for_addr(addr);

        translation_table* table = &root;
        // The last index takes us to the actual page, we'll set that now.
        for (size_t i = 0; i < path.size() - 1; ++i) {
            table = &table->table_at(path[i]);
        }

        (*table)[path.back()].page_num(address_to_page(phys_addr)).valid(true);
        if (type == memory_types::device) {
            (*table)[path.back()]
                .shareable(tos::aarch64::shareable_values::outer)
                .mair_index(PT_DEV);
        } else {
            (*table)[path.back()]
                .shareable(tos::aarch64::shareable_values::inner)
                .mair_index(PT_MEM);
        }
    }

    return {};
}

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const segment& virt_seg) {
    for (uintptr_t addr = virt_seg.range.base; addr != virt_seg.range.end();
         addr += 4096) {
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

expected<const table_entry*, mmu_errors> entry_for_address(const translation_table& root,
                                                           uintptr_t virt_addr) {
    auto path = pt_path_for_addr(virt_addr);

    const translation_table* table = &root;
    // The last index takes us to the actual page, we'll set that now.
    for (size_t i = 0; i < path.size() - 1; ++i) {
        if (!(*table)[i].valid()) {
            return unexpected(mmu_errors::not_allocated);
        }

        table = &table->table_at(path[i]);
    }

    return &(*table)[path.back()];
}

namespace {
bool last_level(int level, const table_entry& entry) {
    if (level == 2)
        return true;
    return !entry.page();
}

expected<translation_table*, mmu_errors> clone_level(int level,
                                                     const translation_table& existing,
                                                     physical_page_allocator& palloc) {
    LOG("Clone with level", level);
    auto table_page = palloc.allocate(1);
    if (!table_page) {
        return unexpected(mmu_errors::page_alloc_fail);
    }
    intrusive_ref(table_page.get());

    EXPECTED_TRYV(allocate_region(
        get_current_translation_table(),
        {{reinterpret_cast<uint64_t>(palloc.address_of(*table_page)), 4096},
         permissions::read_write},
        user_accessible::no,
        &palloc));

    EXPECTED_TRYV(
        mark_resident(get_current_translation_table(),
                      {{reinterpret_cast<uint64_t>(palloc.address_of(*table_page)), 4096},
                       permissions::read_write},
                      memory_types::normal,
                      palloc.address_of(*table_page)));

    auto table_ptr = new (palloc.address_of(*table_page)) translation_table;
    *table_ptr = existing;

    for (auto& entry : *table_ptr) {
        if (entry.valid()) {
            continue;
        }

        if (last_level(level, entry)) {
            continue;
        }

        auto cloned = EXPECTED_TRY(clone_level(level + 1, table_at(entry), palloc));
        entry.page_num(address_to_page(cloned));
    }

    //
    //    for (auto& entry : *table_ptr) {
    //        auto pg_info = palloc.info(entry.page_num());
    //        if (!pg_info) {
    //            continue;
    //        }
    //        intrusive_ref(pg_info);
    //    }

    return table_ptr;
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
} // namespace

expected<translation_table*, mmu_errors>
recursive_table_clone(const translation_table& existing,
                      physical_page_allocator& palloc) {
    return clone_level(0, existing, palloc);
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
} // namespace tos::aarch64