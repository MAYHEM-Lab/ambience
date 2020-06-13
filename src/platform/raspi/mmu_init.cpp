#include <tos/aarch64/mmu.hpp>
#include <arch/detail/bcm2837.hpp>
#include <array>

using tos::aarch64::page_id_t;
using tos::aarch64::table_entry;

// defined in MAIR register
static constexpr auto PT_MEM = 0; // normal memory
static constexpr auto PT_DEV = 1; // device MMIO

extern uint8_t __data_start;

constexpr uintptr_t page_to_address(page_id_t id, size_t page_size = 4096) {
    return id * page_size;
}

constexpr page_id_t address_to_page(uintptr_t ptr, size_t page_size = 4096) {
    return ptr / page_size;
}

page_id_t address_to_page(const volatile void* ptr) {
    return address_to_page(reinterpret_cast<uintptr_t>(ptr));
}

using table = std::array<table_entry, 512>;

struct pages {
    table pgd{}, pud{};
    std::array<table, 4> pmd{};
};

alignas(4096) pages page;

void mmu_init() {
    /**
     * Sets up identity mapping at the first 1 gigabyte of memory.
     */
    auto& l2s = page.pud;
    auto& l3s = page.pmd[0];

    page.pgd[0].zero()
        .page_num(address_to_page(&l2s))
        .valid(true)
        .page(true)
        .accessed(true)
        .shareable(tos::aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    // identity L2 2M blocks
    // skip 0th, as we're about to map it by L3
    for (page_id_t page = 1; page < 512; page++) {
        l2s[page]
            .zero()
            .page_num(address_to_page(page << 21))
            .page(false)
            .accessed(true)
            .noexec(true)
            .valid(true);
        if (page >= address_to_page(bcm2837::IO_BASE) >> 9) {
            l2s[page].shareable(tos::aarch64::shareable_values::outer).mair_index(PT_DEV);
        } else {
            l2s[page].shareable(tos::aarch64::shareable_values::inner).mair_index(PT_MEM);
        }
    }

    l2s[0]
        .zero()
        .page_num(address_to_page(&l3s[0]))
        .valid(true)
        .page(true)
        .accessed(true)
        .shareable(tos::aarch64::shareable_values::inner)
        .mair_index(PT_MEM);

    // identity L3
    for (int r = 0; r < 512; r++) {
        l3s[r]
            .zero()
            .page_num(r)
            .page(true)
            .accessed(true)
            .shareable(tos::aarch64::shareable_values::inner)
            .mair_index(PT_MEM)
            .valid(true);

        /**
         * We follow the W^X permissions scheme.
         *
         * This means that an address is either writeable or executable, but not both at
         * the same time.
         */
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
    r = (1LLU << 37U) |  // Top byte ignore
        (b << 32) |      // IPS=autodetected
        (0b00LL << 14) | // TG0=4k
        (0b11LL << 12) | // SH0=3 inner
        (0b01LL << 10) | // ORGN0=1 write back
        (0b01LL << 8) |  // IRGN0=1 write back
        (0b0LL << 7) |   // EPD0, perform translation on TLB miss
        (25LL << 0);     // T0SZ=25, 3 levels (512G)
    asm volatile("msr tcr_el1, %0; isb" : : "r"(r));

    asm volatile("msr ttbr0_el1, %0" : : "r"(reinterpret_cast<uint64_t>(&page.pgd[0])));

    // finally, toggle some bits in system control register to enable page translation
    asm volatile("dsb ish; isb; mrs %0, sctlr_el1" : "=r"(r));
    r |= 0xC00800;     // set mandatory reserved bits
    r &= ~((1 << 25) | // clear EE, little endian translation tables
           (1 << 24) | // clear E0E
           (1 << 19) | // clear WXN
           (1 << 12) | // clear I, no instruction cache
                       //           (1 << 4) |  // clear SA0
                       //           (1 << 3) |  // clear SA
           (1 << 2) |  // clear C, no cache at all
           (1 << 1));  // clear A, no aligment check
    r |= (1 << 0);     // set M, enable MMU
    asm volatile("msr sctlr_el1, %0; isb" : : "r"(r));
}
