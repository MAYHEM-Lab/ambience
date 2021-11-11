#pragma once

#include "tos/memory.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <tos/expected.hpp>
#include <tos/function_ref.hpp>
#include <tos/paging.hpp>

namespace tos::aarch64 {
static constexpr size_t page_size_bytes = 4096;

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

inline page_id_t address_to_page(virtual_address addr) {
    return address_to_page(addr.address());
}

inline page_id_t address_to_page(physical_address addr) {
    return address_to_page(addr.address());
}

template<uint64_t Pos, uint64_t Len, class Type = uint64_t>
struct bitfield {
    static constexpr uint64_t Position = Pos;
    static constexpr uint64_t Length = Len;
    static constexpr uint64_t Mask = ((1UL << Len) - 1) << Pos;
    using type = Type;
};

static_assert(bitfield<0, 1>::Mask == 0b1);
static_assert(bitfield<1, 1>::Mask == 0b10);
static_assert(bitfield<0, 2>::Mask == 0b11);
static_assert(bitfield<1, 2>::Mask == 0b110);

enum class shareable_values : uint8_t
{
    outer = 0b10,
    inner = 0b11,
};

struct table_entry {
public:
    using Valid = bitfield<0, 1>;
    using Page = bitfield<1, 1>;
    using MAIRIdx = bitfield<2, 3>;
    using AllowUser = bitfield<6, 1>;
    using ReadOnly = bitfield<7, 1>;
    using Shareable = bitfield<8, 2>;
    using Accessed = bitfield<10, 1>;
    using Address = bitfield<12, 36>;
    using NoExec = bitfield<54, 1>;

public:
    [[nodiscard]] constexpr shareable_values shareable() const {
        return shareable_values((m_entry & Shareable::Mask) >> Shareable::Position);
    }

    constexpr table_entry& shareable(shareable_values b) {
        auto tmp = m_entry;
        tmp &= ~Shareable::Mask;
        tmp |= (uint64_t(b) << Shareable::Position) & Shareable::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool allow_user() const {
        return (m_entry & AllowUser::Mask) == AllowUser::Mask;
    }

    constexpr table_entry& allow_user(bool b) {
        auto tmp = m_entry;
        tmp &= ~AllowUser::Mask;
        tmp |= (uint64_t(b) << AllowUser::Position) & AllowUser::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool readonly() const {
        return (m_entry & ReadOnly::Mask) == ReadOnly::Mask;
    }

    constexpr table_entry& readonly(bool b) {
        auto tmp = m_entry;
        tmp &= ~ReadOnly::Mask;
        tmp |= (uint64_t(b) << ReadOnly::Position) & ReadOnly::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool accessed() const {
        return (m_entry & Accessed::Mask) == Accessed::Mask;
    }

    constexpr table_entry& accessed(bool b) {
        auto tmp = m_entry;
        tmp &= ~Accessed::Mask;
        tmp |= (uint64_t(b) << Accessed::Position) & Accessed::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool noexec() const {
        return (m_entry & NoExec::Mask) == NoExec::Mask;
    }

    constexpr table_entry& noexec(bool b) {
        auto tmp = m_entry;
        tmp &= ~NoExec::Mask;
        tmp |= (uint64_t(b) << NoExec::Position) & NoExec::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool valid() const {
        return (m_entry & Valid::Mask) == Valid::Mask;
    }

    constexpr table_entry& valid(bool b) {
        auto tmp = m_entry;
        tmp &= ~Valid::Mask;
        tmp |= (uint64_t(b) << Valid::Position) & Valid::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool page() const {
        return (m_entry & Page::Mask) == Page::Mask;
    }

    constexpr table_entry& page(bool b) {
        auto tmp = m_entry;
        tmp &= ~Page::Mask;
        tmp |= (uint64_t(b) << Page::Position) & Page::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr page_id_t page_num() const {
        return (m_entry & Address::Mask) >> Address::Position;
    }

    constexpr table_entry& page_num(page_id_t id) {
        auto tmp = m_entry;
        tmp &= ~Address::Mask;
        tmp |= (uint64_t(id) << Address::Position) & Address::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr uint8_t mair_index() const {
        return (m_entry & MAIRIdx::Mask) >> MAIRIdx::Position;
    }

    constexpr table_entry& mair_index(uint8_t val) {
        auto tmp = m_entry;
        tmp &= ~MAIRIdx::Mask;
        tmp |= (uint64_t(val) << MAIRIdx::Position) & MAIRIdx::Mask;
        m_entry = tmp;
        return *this;
    }

    constexpr table_entry& zero() {
        m_entry = 0;
        return *this;
    }

    constexpr uint64_t& raw() {
        return m_entry;
    }

    [[nodiscard]] constexpr const uint64_t& raw() const {
        return m_entry;
    }

private:
    uint64_t m_entry;
};
static_assert(sizeof(table_entry) == 8);


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
        return aarch64::table_at((*this)[id]);
    }

    translation_table& table_at(int id) {
        return aarch64::table_at((*this)[id]);
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

translation_table& get_current_translation_table();
translation_table& set_current_translation_table(translation_table& table);

inline void tlb_invalidate_all() {
    dsb();
    asm volatile("tlbi VMALLE1IS");
    dsb();
    isb();
}

enum class mmu_errors
{
    page_alloc_fail,
    already_allocated,
    not_allocated,
    bad_perms,
};

struct vm_page_attributes {
    permissions perms;
    memory_types mem_type;
    bool user_access;
};

expected<void, mmu_errors> allocate_region(translation_table& root,
                                           const virtual_segment& virt_seg,
                                           user_accessible allow_user,
                                           physical_page_allocator* palloc);

expected<void, mmu_errors> mark_resident(translation_table& root,
                                         const virtual_segment& virt_seg,
                                         memory_types type,
                                         physical_address phys_addr);

expected<void, mmu_errors> mark_nonresident(translation_table& root,
                                            const virtual_segment& virt_seg);

expected<const table_entry*, mmu_errors> entry_for_address(const translation_table& root,
                                                           virtual_address virt_addr);

expected<translation_table*, mmu_errors>
recursive_table_clone(const translation_table& existing, physical_page_allocator& palloc);

permissions translate_permissions(const table_entry& entry);

void traverse_table_entries(
    translation_table& table,
    function_ref<void(memory_range vrange, table_entry& entry)> fn);

template<class FnT>
void traverse_table_entries(translation_table& table, FnT&& fn) {
    traverse_table_entries(table, function_ref<void(memory_range, table_entry&)>(fn));
}

inline expected<void, mmu_errors> map_region(translation_table& root,
                                             const virtual_segment& vseg,
                                             user_accessible user_access,
                                             memory_types mem_type,
                                             physical_page_allocator* palloc,
                                             physical_address phys_base) {
    EXPECTED_TRYV(allocate_region(root, vseg, user_access, palloc));

    EXPECTED_TRYV(mark_resident(root, vseg, mem_type, phys_base));

    tos::aarch64::tlb_invalidate_all();

    return {};
}

inline expected<void, mmu_errors>
map_page_ident(translation_table& root,
               physical_page& page,
               physical_page_allocator& palloc,
               permissions perms = permissions::read_write,
               user_accessible user_access = user_accessible::no,
               memory_types mem_type = memory_types::normal) {
    const auto physical_seg =
        physical_segment{.range = palloc.range_of(page), .perms = perms};
    return map_region(root,
                      identity_map(physical_seg),
                      user_access,
                      mem_type,
                      &palloc,
                      physical_seg.range.base);
}

} // namespace tos::aarch64
