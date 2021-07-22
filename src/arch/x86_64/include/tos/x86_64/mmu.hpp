#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/memory.hpp>
#include <tos/paging/physical_page_allocator.hpp>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
constexpr auto page_size_bytes = 4096;
constexpr auto page_size_log = 12;

using page_id_t = uint32_t;

constexpr uintptr_t page_to_address(page_id_t id, size_t page_size = page_size_bytes) {
    return id * page_size;
}

constexpr page_id_t address_to_page(uintptr_t ptr, size_t page_size = page_size_bytes) {
    return ptr / page_size;
}

inline page_id_t address_to_page(const volatile void* ptr) {
    return address_to_page(reinterpret_cast<uintptr_t>(ptr));
}

struct table_entry {
public:
    table_entry& zero() {
        m_raw_entry = 0;
        return *this;
    }

    bool readonly() const {
        return !writeable();
    }

    bool writeable() const {
        return m_raw_entry & writeable_mask;
    }

    table_entry& writeable(bool write) {
        m_raw_entry = (m_raw_entry & ~writeable_mask) | (write << writeable_off);
        return *this;
    }

    bool valid() const {
        return m_raw_entry & present_mask;
    }

    table_entry& valid(bool p) {
        m_raw_entry = (m_raw_entry & ~present_mask) | (p << present_off);
        return *this;
    }

    bool huge_page() const {
        return m_raw_entry & huge_page_mask;
    }

    table_entry& huge_page(bool huge) {
        m_raw_entry = (m_raw_entry & ~huge_page_mask) | (huge << huge_page_off);
        return *this;
    }

    uintptr_t page_num() const {
        return (m_raw_entry & page_base_mask) >> page_base_off;
    }

    table_entry& page_num(uintptr_t base) {
        m_raw_entry = (m_raw_entry & ~page_base_mask) | (base & page_base_mask);
        return *this;
    }

    bool allow_user() const {
        return m_raw_entry & user_access_mask;
    }

    table_entry& allow_user(bool val) {
        m_raw_entry = (m_raw_entry & ~user_access_mask) | (val << user_access_off);
        return *this;
    }

    bool noexec() const {
        return m_raw_entry & noexec_mask;
    }

    table_entry& noexec(bool val) {
        m_raw_entry = (m_raw_entry & ~noexec_mask) | (uint64_t(val ? 1 : 0) << noexec_off);
        return *this;
    }

    uint64_t raw() const {
        return m_raw_entry;
    }

private:
    uint64_t m_raw_entry;
    static constexpr auto present_off = 0U;
    static constexpr auto writeable_off = 1U;
    static constexpr auto user_access_off = 2U;
    static constexpr auto huge_page_off = 7U;
    static constexpr auto noexec_off = 63U;
    static constexpr auto page_base_off = 12U;

    static constexpr auto present_mask = 0x1ULL << present_off;
    static constexpr auto writeable_mask = 0x1ULL << writeable_off;
    static constexpr auto user_access_mask = 0x1ULL << user_access_off;
    static constexpr auto huge_page_mask = 0x1ULL << huge_page_off;
    static constexpr auto noexec_mask = 0x1ULL << noexec_off;
    static constexpr auto page_base_mask = 0x00'FF'FF'FF'FF'FF'FULL << page_base_off;
};

struct translation_table;
inline translation_table& table_at(table_entry& entry) {
    return *reinterpret_cast<translation_table*>(page_to_address(entry.page_num()));
}

inline const translation_table& table_at(const table_entry& entry) {
    return *reinterpret_cast<const translation_table*>(page_to_address(entry.page_num()));
}

struct alignas(4096) translation_table {
    table_entry& operator[](int id) {
        return entries[id];
    }

    const table_entry& operator[](int id) const {
        return entries[id];
    }

    const translation_table& table_at(int id) const {
        return x86_64::table_at((*this)[id]);
    }

    translation_table& table_at(int id) {
        return x86_64::table_at((*this)[id]);
    }

    size_t size() const {
        return entries.size();
    }

    auto begin() {
        return entries.begin();
    }

    auto end() {
        return entries.end();
    }

private:
    std::array<table_entry, 512> entries;
};

void traverse_table_entries(
    translation_table& table,
    function_ref<void(memory_range vrange, table_entry& entry)> fn);

template<class FnT>
void traverse_table_entries(translation_table& table, FnT&& fn) {
    traverse_table_entries(table, function_ref<void(memory_range, table_entry&)>(fn));
}

translation_table& get_current_translation_table();
permissions translate_permissions(const table_entry& entry);

enum class mmu_errors
{
    page_alloc_fail,
    already_allocated,
    not_allocated,
    bad_perms,
};

expected<void, mmu_errors> allocate_region(translation_table& root,
                                           const segment& virt_seg,
                                           user_accessible allow_user,
                                           physical_page_allocator* palloc);

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const memory_range& range,
                                         memory_types type,
                                         void* phys_addr);

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const memory_range& virt_range);

expected<const table_entry*, mmu_errors> entry_for_address(const translation_table& root,
                                                           uintptr_t virt_addr);

inline expected<void, mmu_errors> map_region(translation_table& root,
                                             const segment& vseg,
                                             user_accessible user_access,
                                             memory_types mem_type,
                                             physical_page_allocator* palloc,
                                             void* phys_base) {
    EXPECTED_TRYV(allocate_region(root, vseg, user_access, palloc));

    EXPECTED_TRYV(mark_resident(root, vseg.range, mem_type, phys_base));

    return {};
}

NO_INLINE
inline void tlb_flush() {
    tos::x86_64::write_cr3(tos::x86_64::read_cr3());
}
} // namespace tos::x86_64