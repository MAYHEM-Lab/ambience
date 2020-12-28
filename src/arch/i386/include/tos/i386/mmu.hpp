#pragma once

#include <cstdint>
#undef i386

namespace tos::i386 {
struct gdt {};

struct page_entry {
public:
    bool writeable() const {
        return m_raw_entry & writeable_mask;
    }

    void writeable(bool write) {
        m_raw_entry = (m_raw_entry & ~writeable_mask) | (write << 1);
    }

    void present(bool p) {
        m_raw_entry = (m_raw_entry & ~present_mask) | p;
    }

    bool present() const {
        return m_raw_entry & present_mask;
    }

    bool huge_page() const {
        return m_raw_entry & huge_page_mask;
    }

    void huge_page(bool huge) {
        m_raw_entry = (m_raw_entry & ~huge_page_mask) | (huge << 7);
    }

    uintptr_t page_base() const {
        return m_raw_entry & 0xFF'FF'FF'FF'FF'FF'F0'00;
    }

    void page_base(uintptr_t base) {
        m_raw_entry = (m_raw_entry & ~page_base_mask) | (base & page_base_mask);
    }

private:
    uint64_t m_raw_entry;

    static constexpr auto present_mask = 0x1 << 0;
    static constexpr auto writeable_mask = 0x1 << 1;
    static constexpr auto huge_page_mask = 0x1 << 7;
    static constexpr auto page_base_mask = 0xFF'FF'FF'FF'FF'FF'F0'00;
};
} // namespace tos::i386