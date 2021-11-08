#pragma once

#include <array>
#include <tos/x86_64/mmu/common.hpp>
#include <tos/x86_64/mmu/table_entry.hpp>

namespace tos::x86_64 {
struct translation_table;
inline translation_table& table_at(table_entry& entry) {
    return *reinterpret_cast<translation_table*>(page_to_address(entry.page_num()));
}

inline const translation_table& table_at(const table_entry& entry) {
    return *reinterpret_cast<const translation_table*>(page_to_address(entry.page_num()));
}

struct alignas(4096) translation_table {
    constexpr table_entry& operator[](int id) {
        return entries[id];
    }

    constexpr const table_entry& operator[](int id) const {
        return entries[id];
    }

    constexpr const translation_table& table_at(int id) const {
        return x86_64::table_at((*this)[id]);
    }

    constexpr translation_table& table_at(int id) {
        return x86_64::table_at((*this)[id]);
    }

    constexpr size_t size() const {
        return entries.size();
    }

    constexpr auto begin() const {
        return entries.begin();
    }

    constexpr auto end() const {
        return entries.end();
    }

    constexpr auto begin() {
        return entries.begin();
    }

    constexpr auto end() {
        return entries.end();
    }

private:
    // Zero initialized
    std::array<table_entry, 512> entries{};
};
} // namespace tos::x86_64