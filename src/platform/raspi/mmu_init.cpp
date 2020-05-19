#include <arch/detail/bcm2837.hpp>
#include <array>

static constexpr auto PageSize = 4096;

// defined in MAIR register
static constexpr auto PT_MEM = 0; // normal memory
static constexpr auto PT_DEV = 1; // device MMIO

extern uint8_t __data_start;

using page_id = int32_t;
constexpr uintptr_t page_to_address(page_id id) {
    return id * PageSize;
}

constexpr page_id address_to_page(uintptr_t ptr) {
    return ptr / PageSize;
}

page_id address_to_page(const volatile void* ptr) {
    return address_to_page(reinterpret_cast<uintptr_t>(ptr));
}

namespace aarch64 {
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
        tmp |= (uint8_t(b) << Shareable::Position) & Shareable::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool readonly() const {
        return (m_entry & ReadOnly::Mask) == ReadOnly::Mask;
    }

    constexpr table_entry& readonly(bool b) {
        auto tmp = m_entry;
        tmp &= ~ReadOnly::Mask;
        tmp |= (b << ReadOnly::Position) & ReadOnly::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool accessed() const {
        return (m_entry & Accessed::Mask) == Accessed::Mask;
    }

    constexpr table_entry& accessed(bool b) {
        auto tmp = m_entry;
        tmp &= ~Accessed::Mask;
        tmp |= (b << Accessed::Position) & Accessed::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool noexec() const {
        return (m_entry & NoExec::Mask) == NoExec::Mask;
    }

    constexpr table_entry& noexec(bool b) {
        auto tmp = m_entry;
        tmp &= ~NoExec::Mask;
        tmp |= (b << NoExec::Position) & NoExec::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool valid() const {
        return (m_entry & Valid::Mask) == Valid::Mask;
    }

    constexpr table_entry& valid(bool b) {
        auto tmp = m_entry;
        tmp &= ~Valid::Mask;
        tmp |= (b << Valid::Position) & Valid::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr bool page() const {
        return (m_entry & Page::Mask) == Page::Mask;
    }

    constexpr table_entry& page(bool b) {
        auto tmp = m_entry;
        tmp &= ~Page::Mask;
        tmp |= (b << Page::Position) & Page::Mask;
        m_entry = tmp;
        return *this;
    }

    [[nodiscard]] constexpr page_id page_num() const {
        return (m_entry & Address::Mask) >> Address::Position;
    }

    constexpr table_entry& page_num(page_id id) {
        auto tmp = m_entry;
        tmp &= ~Address::Mask;
        tmp |= (id << Address::Position) & Address::Mask;
        m_entry = tmp;
        return *this;
    }

    constexpr uint8_t mair_index() const {
        return (m_entry & MAIRIdx::Mask) >> MAIRIdx::Position;
    }

    constexpr table_entry& mair_index(uint8_t val) {
        auto tmp = m_entry;
        tmp &= ~MAIRIdx::Mask;
        tmp |= (val << MAIRIdx::Position) & MAIRIdx::Mask;
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

    constexpr const uint64_t& raw() const {
        return m_entry;
    }

private:
    uint64_t m_entry;
};
static_assert(sizeof(table_entry) == 8);
} // namespace aarch64

using table = std::array<aarch64::table_entry, 512>;

struct pages {
    table pgd{}, pud{};
    std::array<table, 4> pmd{};
};

[[gnu::section(".pagetables")]] alignas(4096) pages page;

void mmu_init() {
    auto& root = page.pgd[0];
    auto& l2s = page.pud;
    auto& l3s = page.pmd[0];

    root.zero()
        .page_num(address_to_page(&l2s))
        .valid(true)
        .page(true)
        .accessed(true)
        .shareable(aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    // identity L2 2M blocks
    // skip 0th, as we're about to map it by L3
    for (int page = 1; page < 512; page++) {
        l2s[page]
            .zero()
            .page_num(address_to_page(page << 21))
            .page(false)
            .accessed(true)
            .noexec(true)
            .valid(true);
        if (page >= (bcm2837::IO_BASE >> 21)) {
            l2s[page].shareable(aarch64::shareable_values::outer).mair_index(PT_DEV);
        } else {
            l2s[page].shareable(aarch64::shareable_values::inner).mair_index(PT_MEM);
        }
    }

    l2s[0]
        .zero()
        .page_num(address_to_page(&l3s[0]))
        .valid(true)
        .page(true)
        .accessed(true)
        .shareable(aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    // identity L3
    for (int r = 0; r < 512; r++)
    {
        l3s[r]
            .zero()
            .page_num(address_to_page(r * PageSize))
            .page(true)
            .accessed(true)
            .shareable(aarch64::shareable_values::inner)
            .mair_index(PT_MEM)
            .valid(true);
        if (r < 0x80 || r >= address_to_page(&__data_start)) {
            l3s[r].noexec(true);
        } else {
            l3s[r].readonly(true);
        }
    }

    // check for 4k granule and at least 36 bits physical address bus */
    uint64_t r;
    asm volatile("mrs %0, id_aa64mmfr0_el1" : "=r"(r));
    auto b = r & 0xF;
    if (r & (0xF << 28) /*4k*/ || b < 1 /*36 bits*/) {
        return;
    }

    // Memory Attributes array, indexed by PT_MEM, PT_DEV
    r = (0xFF << 0) | // AttrIdx=0: normal, IWBWA, OWBWA, NTR
        (0x04 << 8);  // AttrIdx=1: device, nGnRE (must be OSH too);
    asm volatile("msr mair_el1, %0" : : "r"(r));

    // https://developer.arm.com/docs/ddi0595/b/aarch64-system-registers/tcr_el1
    r = (1 << 37) |      // Top byte ignore
        (b << 32) |      // IPS=autodetected
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL << 7) |   // EPD0 enable lower half
        (25LL << 0);     // T0SZ=25, 3 levels (512G)
    asm volatile("msr tcr_el1, %0; isb" : : "r"(r));

    asm volatile("msr ttbr0_el1, %0" : : "r"((unsigned long)&root));

    // finally, toggle some bits in system control register to enable page translation
    asm volatile("dsb ish; isb; mrs %0, sctlr_el1" : "=r"(r));
    r |= 0xC00800;     // set mandatory reserved bits
    r &= ~((1 << 25) | // clear EE, little endian translation tables
           (1 << 24) | // clear E0E
           (1 << 19) | // clear WXN
           (1 << 12) | // clear I, no instruction cache
           (1 << 4) |  // clear SA0
           (1 << 3) |  // clear SA
           (1 << 2) |  // clear C, no cache at all
           (1 << 1));  // clear A, no aligment check
    r |= (1 << 0);     // set M, enable MMU
    asm volatile("msr sctlr_el1, %0; isb" : : "r"(r));
}
