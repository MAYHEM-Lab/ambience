#pragma once

#include <cstdint>

namespace tos::x86_64 {
struct table_entry {
public:
    constexpr table_entry& zero() {
        m_raw_entry = 0;
        return *this;
    }

    constexpr bool readonly() const {
        return !writeable();
    }

    constexpr bool writeable() const {
        return m_raw_entry & writeable_mask;
    }

    constexpr table_entry& writeable(bool write) {
        m_raw_entry = (m_raw_entry & ~writeable_mask) | (write << writeable_off);
        return *this;
    }

    constexpr bool valid() const {
        return m_raw_entry & present_mask;
    }

    constexpr table_entry& valid(bool p) {
        m_raw_entry = (m_raw_entry & ~present_mask) | (p << present_off);
        return *this;
    }

    constexpr bool huge_page() const {
        return m_raw_entry & huge_page_mask;
    }

    constexpr table_entry& huge_page(bool huge) {
        m_raw_entry = (m_raw_entry & ~huge_page_mask) | (huge << huge_page_off);
        return *this;
    }

    constexpr uintptr_t page_num() const {
        return (m_raw_entry & page_base_mask) >> page_base_off;
    }

    constexpr table_entry& page_num(uintptr_t base) {
        m_raw_entry = (m_raw_entry & ~page_base_mask) | (base & page_base_mask);
        return *this;
    }

    constexpr bool allow_user() const {
        return m_raw_entry & user_access_mask;
    }

    constexpr table_entry& allow_user(bool val) {
        m_raw_entry = (m_raw_entry & ~user_access_mask) | (val << user_access_off);
        return *this;
    }

    constexpr bool noexec() const {
        return m_raw_entry & noexec_mask;
    }

    constexpr table_entry& noexec(bool val) {
        m_raw_entry =
            (m_raw_entry & ~noexec_mask) | (uint64_t(val ? 1 : 0) << noexec_off);
        return *this;
    }

    constexpr uint64_t raw() const {
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
}