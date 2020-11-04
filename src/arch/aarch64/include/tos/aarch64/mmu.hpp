#pragma once

#include <cstddef>
#include <cstdint>

namespace tos::aarch64 {
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

    [[nodiscard]]
    constexpr uint8_t mair_index() const {
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

    [[nodiscard]]
    constexpr const uint64_t& raw() const {
        return m_entry;
    }

private:
    uint64_t m_entry;
};
static_assert(sizeof(table_entry) == 8);
}
