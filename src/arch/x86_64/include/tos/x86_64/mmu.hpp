#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <tos/function_ref.hpp>
#include <tos/memory.hpp>
#include <tos/x86_64/assembly.hpp>

namespace tos::x86_64 {
using page_id_t = uint32_t;

constexpr uintptr_t page_to_address(page_id_t id, size_t page_size = 4096) {
    return id * page_size;
}

constexpr page_id_t address_to_page(uintptr_t ptr, size_t page_size = 4096) {
    return ptr / page_size;
}

inline page_id_t address_to_page(const volatile void* ptr) {
    return address_to_page(reinterpret_cast<uintptr_t>(ptr));
}

struct table_entry {
public:
    bool writeable() const {
        return m_raw_entry & writeable_mask;
    }

    bool readonly() const {
        return !writeable();
    }

    table_entry& writeable(bool write) {
        m_raw_entry = (m_raw_entry & ~writeable_mask) | (write << 1);
        return *this;
    }

    table_entry& valid(bool p) {
        m_raw_entry = (m_raw_entry & ~present_mask) | p;
        return *this;
    }

    bool valid() const {
        return m_raw_entry & present_mask;
    }

    bool huge_page() const {
        return m_raw_entry & huge_page_mask;
    }

    table_entry& huge_page(bool huge) {
        m_raw_entry = (m_raw_entry & ~huge_page_mask) | (huge << 7);
        return *this;
    }

    uintptr_t page_num() const {
        return (m_raw_entry & 0xFF'FF'FF'FF'FF'FF'F0'00) >> 12;
    }

    table_entry& page_num(uintptr_t base) {
        m_raw_entry = (m_raw_entry & ~page_base_mask) | (base & page_base_mask);
        return *this;
    }

    table_entry& zero() {
        m_raw_entry = 0;
        return *this;
    }

    bool allow_user() const {
        return m_raw_entry & user_access_mask;
    }

    bool noexec() const {
        return m_raw_entry & noexec_mask;
    }
    uint64_t m_raw_entry;

private:
    static constexpr auto present_mask = 0x1 << 0;
    static constexpr auto writeable_mask = 0x1 << 1;
    static constexpr auto user_access_mask = 0x1 << 2;
    static constexpr auto huge_page_mask = 0x1 << 7;
    static constexpr auto noexec_mask = 0x1ULL << 63;
    static constexpr auto page_base_mask = 0xFF'FF'FF'FF'FF'FF'F0'00;
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
} // namespace tos::x86_64