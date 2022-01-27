#pragma once

#include <cstdint>
#include <cstddef>
#include <tos/memory.hpp>

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

inline page_id_t address_to_page(physical_address addr) {
    return address_to_page(addr.address());
}

struct translation_table;
struct table_entry;

translation_table& get_current_translation_table();
permissions translate_permissions(const table_entry& entry);
} // namespace tos::x86_64